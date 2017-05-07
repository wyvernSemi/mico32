#!/usr/bin/env python3
# =======================================================================
#                                                                        
#  lm32.py                                             date: 2017/03/07
#                                                                        
#  Author: Simon Southwell                                               
# 
#  Copyright (c) 2017 Simon Southwell 
#
#  GUI front end for cpumico32 and lnxmico32.
#                                                                        
#  This file is part of the cpumico32 instruction set simulator.
#  
#  This file is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#  
#  The code is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this file. If not, see <http://www.gnu.org/licenses/>.
#  
#  $Id: lm32.py,v 1.8 2017/04/20 08:51:34 simon Exp $
#  $Source: /home/simon/CVS/src/cpu/mico32/python/lm32.py,v $
#                                                                       
# =======================================================================

# =======================================================================
#
# lm32gui main call hierarchy:
#
#  run
#    __lm32CreateWidgets()                   --- Creates top level, menu, notebook (with two tabs) and runs main loop
#      __lm32CreateSubWidgets()              --- For each notebook tab creates checkbuttons, entries and buttons
#        wyTkinterUtils.addCheckButtonRows() --- static. Utility to add checkbuttons based on list of tuples
#        wyTkinterUtils.addEntryRows()       --- static. Utility to add Entry widgets based on list of tuples
#        wyTkinterUtils.addFileEntryRows()   --- static. Utility to add File entry widgets based on list of tuples
#
# lm32gui callback methods (and hierarchy):
#
# Menus:
#  __lm32OpenFile()                          --- File->Open Elf File...
#  __lm32OpenExecFolder()                    --- File->Executable Folder...
#  __lm32OpenRunFolder()                     --- File->Change Run Folder...
#  __lm32About()                             --- Help->About...
#
# Browse Buttons:
#  __lm32GetIni()                            --- Opens file dialog to specify .ini file
#  __lm32GetLog()                            --- Opens file dialog to specify log file
#  __lm32GetSav()                            --- Opens file dialog to specify save state file
#
# Tk variable updates:
#  __lm32SaveStateUpdated()                  --- savestate or loadstate Tk variables updated callback
#  __lm32DumpAddrUpdated()                   --- dumpaddr Tk variable update callback
#  __lm32TcpUpdated()                        --- enabletcp Tk variable update callback
#  __lm32CfgUpdated()                        --- Any of the CFG field Tk variables updated callback
#  __lm32fastmodeUpdated()                   --- fastmode Tk variable update callback
#
# Execution buttons:
#  __lm32RunCmd()                            --- Run button callback to execute program
#    lm32GuiBase._lm32CheckStringNum()       --- static. Utility to ensure entry's a valid, in range, number
#    lm32GuiBase._lm32ShellCmd()             --- static. Utility to execute a command based on a string.
#  __lm32DebugCmd()                          --- Debug button callback that calls __lm32RunCmd() in debug mode.
#    __lm32RunCmd()
#
# =======================================================================

# Get libraries for interfacing with the OS
import os, subprocess

# Get everything from Tkinter, as we use quite a lot
from tkinter      import *

# Override any Tk widgets that have themed versions in ttk,
# and pull in other ttk specific widgets (ie. Separator and Notebook)
from tkinter.ttk  import *

# Only promote what's used from the support libraries
from tkinter.filedialog import askopenfilename, askdirectory
from tkinter.messagebox import showinfo, showerror

# ----------------------------------------------------------------
# Define lm32Stdout class to replace standard output IO.
# ----------------------------------------------------------------

class lm32Stdout(object) :

  # Constructor: save a handle to given textbox object
  def __init__(self, txtwidget):
    self.text_space = txtwidget

  # Need a write method
  def write(self, string):
    # Add the string being written to trhe end of the text
    self.text_space.insert('end', string)

    # Make sure end of the newly updated text is visible
    self.text_space.see(END)

  # Need a flush method to stop exceptions (it is called on closure),
  # but it doesn't need to do anything
  def flush(self):
    pass

# ----------------------------------------------------------------
# Define generic WyvernSemi Tkinter utils in a class
# ----------------------------------------------------------------

class wyTkinterUtils :

  # addCheckButtonRows()
  #
  # Creates an array of labelled check button widgets. Widgets are
  # children of 'frame' and a label for the frame is created from
  # 'labeltxt'. The tupleList should be a two dimensional array
  # ([row][col]) of tuples of the form:
  #  (<label txt>, <Tk variable>, <onvalue>)
  #
  # Returns list of widget handles as a linear list (scanning from left to right,
  # then top to bottom)

  @staticmethod
  def addCheckButtonRows(labeltxt, tupleList, frame) :

    curr_row = 0

    # Add a label for this frame, if one specified
    if labeltxt != '' :
      Label(frame, text=labeltxt).grid(row=curr_row, sticky=W)
      curr_row += 1

    # Initialise the handle list
    hdllist = []

    # Loop through the list of rows
    for rowList in tupleList :

      curr_col = 0

      # Loop through the column tuples in the row list, extracting a
      # label text string, and a Tk variable
      for txt, var, onvalue in rowList :

        # Create a check button and specify grid position
        hdl = Checkbutton(master = frame, text = txt, variable = var, onvalue = onvalue)
        hdl.grid(row = curr_row, column = curr_col, sticky = W, padx = 10)

        # Add handle to list
        hdllist.append(hdl)

        curr_col += 1

      curr_row += 1

    # When done, return list of widget handles as a linear list
    # (scanning from left to right, then top to bottom)
    return hdllist

  # addEntryRows()
  #
  # Creates an array of labelled entry widgets. Widgets are children of 'frame'
  # and a label for the frame is created from 'labeltxt'. The tupleList should
  # be a two dimensional array ([row][col]) of tuples of the form:
  #  (<label txt>, <Tk variable>, <entry width>)
  #
  @staticmethod
  def addEntryRows(labeltxt, tupleList, frame) :

    curr_row = 0

    # Add a label for this frame
    if labeltxt != '' :
      Label(master = frame, text = labeltxt).grid(row = curr_row, sticky = W)
      curr_row += 1

    # Start a new row, and initialise the handle list

    hdllist = []

    # Loop through the list of rows
    for rowList in tupleList :

      curr_col = 0

      # Loop through the column tuples in the row list, extracting a
      # label text string, a Tk variable, and a width value
      for txt, var, width in rowList :

        # Create a labelled entry widget and specify grid position
        Label(master = frame, text = txt).grid (row = curr_row, column = curr_col, sticky = E, pady = 5, padx = 5)
        curr_col += 1
        hdl = Entry(master = frame, textvariable = var, width = width)
        hdl.grid(row = curr_row, column = curr_col, sticky = W)

        # Add handle to list
        hdllist.append(hdl)

        curr_col += 1

      curr_row += 1

    # When done, return list of widget handles as a linear list
    # (scanning from left to right then top to bottom)
    return hdllist


  # addFileEntryRows()
  #
  # Creates an array of labelled entry widgets with a folloiwng 'Browse' button.
  # Widgets are children of 'frame' and a label for the frame is created from 'labeltxt'.
  # tupleList should be a two dimensional array ([rows][cols]) of tuples
  # of the form:
  #    (<label txt>, <Tk variable>, <entry width>, <button txt>, <button callback>)
  # If button callback is None, then no button is added.
  #
  @staticmethod
  def addFileEntryRows(labeltxt, tupleList, frame) :

    curr_row = 0

    # Add a label for this frame, if one specified
    if labeltxt != '' :
      Label(master = frame, text = labeltxt).grid(row = curr_row, sticky = W, pady = 0)
      curr_row += 1

    # Initialise the handle list
    hdllist = []

    # Loop through the list of rows
    for rowList in tupleList :

      curr_col = 0

      # Loop through the column tuples in the row list, extracting a
      # label text string, a Tk variable, a width value, button label text
      # and a button callback function
      for txt, var, width, btxt, bfunc in rowList :

        # Create a labelled entry widget and specify grid position
        Label(master = frame, text=txt).grid (row = curr_row, column = curr_col, sticky = E, pady = 5, padx = 5)
        curr_col += 1
        hdl = Entry(frame, textvariable = var, width = width)
        hdl.grid(row = curr_row, column = curr_col, sticky = W)

        # Add entry handle to handle list
        hdllist.append(hdl)

        if bfunc is not None :
          # In adjacent column, add a 'Browse' button
          curr_col += 1
          hdl = Button(master = frame, text = btxt, command = bfunc)
          hdl.grid (row = curr_row, column = curr_col, sticky = W, padx = 10)

          # Add handle for button to handle list
          hdllist.append(hdl)
          curr_col += 1

      curr_row += 1

    # When done, return list of widget handles as a linear list
    # (scanning from left to right---Entry then Button widgets---then
    # top to bottom)
    return hdllist

# ----------------------------------------------------------------
# Define the lm32gui class constants and utility functions in a
# base class
# ----------------------------------------------------------------

class lm32GuiBase :

  # Defaults for the settings. Defined here for comparison
  # when generating a command line, so as not to output a
  # command line option if value is at its default value.
  # NB: meant to be READ ONLY.

  _LM32DEFAULTelfname            = 'test.elf'
  _LM32DEFAULTgdb                = 0
  _LM32DEFAULTdumpregs           = 0
  _LM32DEFAULTdumpnuminstr       = 0
  _LM32DEFAULTdisassemble        = 0
  _LM32DEFAULTverbose            = 0
  _LM32DEFAULTenablecb           = 0
  _LM32DEFAULTdisablebreakonlock = 0
  _LM32DEFAULTenabletcp          = 0
  _LM32DEFAULTmemsize            = '65536'
  _LM32DEFAULTmemoffset          = '0'
  _LM32DEFAULTmemaddr            = '0'
  _LM32DEFAULTnuminstr           = 'forever'
  _LM32DEFAULTdumpaddr           = 'NONE'
  _LM32DEFAULTdumpbytes          = '4'
  _LM32DEFAULTcfgword            = '0x011203f7'
  _LM32DEFAULTwaitstates         = '0'
  _LM32DEFAULTbpaddr             = 'NONE'
  _LM32DEFAULTinifile            = 'NONE'
  _LM32DEFAULTlogfile            = 'stdout'
  _LM32DEFAULTexecfolder         = '<PATH>'
  _LM32DEFAULTcomport            = '6'
  _LM32DEFAULTtcpport            = '49152' # First non-registerable port number (2^15 + 2^14)
  _LM32DEFAULTfastmode           = 0
  _LM32DEFAULTsavestate          = 0
  _LM32DEFAULTloadstate          = 0
  _LM32DEFAULTsavfilename        = 'lnxmico32.sav'

  # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  # Utility functions
  # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  # _lm32ShellCmd()
  #
  # Run a command in a shell using subprocess
  #
  # Args:
  #  cmd_str    : String containing command to be run in OS
  #  curr_dir   : String indicating directory from which command is to be run
  #  print_only : Flag indicating to print commands rather than execute
  #  cd         : Flag indicating, if printing, whether to output a cd command
  #  background : Flag indicating to run command in the background
  #

  @staticmethod
  def _lm32ShellCmd (cmd_str, curr_dir = None, islnx = False) :

    # Run a process for the command
    if not islnx :
      proc = subprocess.Popen(args               = cmd_str,
                              shell              = True,
                              cwd                = curr_dir,
                              stdout             = subprocess.PIPE,
                              #stderr             = subprocess.PIPE,
                              universal_newlines = True)
    else :
      proc = subprocess.Popen(args               = cmd_str,
                              shell              = True,
                              cwd                = curr_dir,
                              universal_newlines = True)

    # Get the output from the process after the process terminates
    rtn_stdout, rtn_stderr = proc.communicate()

    print(rtn_stdout)

  # _lm32CheckStringNum()
  #
  # Check if a string is an integer number, and between specified ranges
  #
  # noinspection PyBroadException
  @staticmethod
  def _lm32CheckStringNum(numstr, minval, maxval, rtnlist) :
    # First check the string is an integer number
    try:
      v = int(numstr, 0)
    except:
      return False

    # Now check the range
    if (minval != 0 or maxval != 0) and (v < minval or v > maxval) :
      return False

    rtnlist[0] = v

    return True

# ----------------------------------------------------------------
# Define the lm32gui class
# ----------------------------------------------------------------

# noinspection PyUnusedLocal,PySimplifyBooleanCheck,PyShadowingBuiltins,PyBroadException
class lm32gui (lm32GuiBase, wyTkinterUtils):

  # --------------------------------------------------------------
  # Define the class variables (i.e. like C/C++ static variables)
  # --------------------------------------------------------------

  # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  # Constructor
  # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  def __init__(self) :
  
    # ------------------------------------------------------------
    # Create the instance variables here (i.e. like C++ member 
    # variables)
    # ------------------------------------------------------------
  
    # Get a Tk object
    self.root = Tk()
    
    # Create some Tk class variables to allow auto-update of
    # widgets. The 'boolean' checkbutton vars are integers,
    # with the rest as strings (even if meant for numbers)
    self.gdb                = IntVar()
    self.dumpregs           = IntVar()
    self.dumpnuminstr       = IntVar()
    self.disassemble        = IntVar()
    self.verbose            = IntVar()
    self.enablecb           = IntVar()
    self.disablebreakonlock = IntVar()
    self.enabletcp          = IntVar()
    self.savestate          = IntVar()
    self.loadstate          = IntVar()
    self.savfilename        = StringVar()
    self.comport            = StringVar()
    self.memsize            = StringVar()
    self.memoffset          = StringVar()
    self.memaddr            = StringVar()
    self.numinstr           = StringVar()
    self.dumpaddr           = StringVar()
    self.dumpbytes          = StringVar()
    self.cfgword            = StringVar()
    self.waitstates         = StringVar()
    self.bpaddr             = StringVar()
    self.inifile            = StringVar()
    self.logfile            = StringVar()
    self.elfname            = StringVar()
    self.execfolder         = StringVar()
    
    # Individual CFG flags, with on value their bit mask
    self.mflag              = IntVar()
    self.dflag              = IntVar()
    self.sflag              = IntVar()   
    self.xflag              = IntVar()
    self.ccflag             = IntVar()    
    self.dcflag             = IntVar()
    self.icflag             = IntVar()
    self.gflag              = IntVar()
    self.hflag              = IntVar()
    self.rflag              = IntVar()
    self.jflag              = IntVar()
    
    self.cfgint             = StringVar()
    self.cfgbp              = StringVar()
    self.cfgwp              = StringVar()

    # Tk variables for directory locations
    self.scriptdir          = StringVar()
    self.rundir             = StringVar()

    # ------------------------------------------------------------
    # Set/configure instance objects here
    # ------------------------------------------------------------
    
    self.root.title('lm32.py : Copyright (c) 2017 WyvernSemi')
    
    # Configure the font for message boxes (the default is awful)
    self.root.option_add('*Dialog.msg.font', 'Ariel 10')

    # Set some location state
    self.scriptdir.set(os.path.dirname(os.path.realpath(sys.argv[0])))
    self.rundir.set(os.getcwd())

    # Change the default icon to something more appropriate
    self.__lm32SetIcon(self.root)
   
    # Set the Tk variables to default values
    self.gdb.set                (lm32gui._LM32DEFAULTgdb               )
    self.elfname.set            (lm32gui._LM32DEFAULTelfname           )
    self.dumpregs.set           (lm32gui._LM32DEFAULTdumpregs          )
    self.dumpnuminstr.set       (lm32gui._LM32DEFAULTdumpnuminstr      )
    self.disassemble.set        (lm32gui._LM32DEFAULTdisassemble       )
    self.verbose.set            (lm32gui._LM32DEFAULTverbose           )
    self.enablecb.set           (lm32gui._LM32DEFAULTenablecb          )
    self.disablebreakonlock.set (lm32gui._LM32DEFAULTdisablebreakonlock)
    self.enabletcp.set          (lm32gui._LM32DEFAULTenabletcp         )
    self.comport.set            (lm32gui._LM32DEFAULTcomport           )
    self.memsize.set            (lm32gui._LM32DEFAULTmemsize           )
    self.memoffset.set          (lm32gui._LM32DEFAULTmemoffset         )
    self.memaddr.set            (lm32gui._LM32DEFAULTmemaddr           )
    self.numinstr.set           (lm32gui._LM32DEFAULTnuminstr          )
    self.dumpaddr.set           (lm32gui._LM32DEFAULTdumpaddr          )
    self.dumpbytes.set          (lm32gui._LM32DEFAULTdumpbytes         )
    self.cfgword.set            (lm32gui._LM32DEFAULTcfgword           )
    self.waitstates.set         (lm32gui._LM32DEFAULTwaitstates        )
    self.bpaddr.set             (lm32gui._LM32DEFAULTbpaddr            )
    self.inifile.set            (lm32gui._LM32DEFAULTinifile           )
    self.logfile.set            (lm32gui._LM32DEFAULTlogfile           )
    self.execfolder.set         (lm32gui._LM32DEFAULTexecfolder        )
    self.savestate.set          (lm32gui._LM32DEFAULTsavestate         )
    self.loadstate.set          (lm32gui._LM32DEFAULTloadstate         )
    self.savfilename.set        (lm32gui._LM32DEFAULTsavfilename       )
    
    # Bind a callback for dump address changes (i.e. is written to),
    # for use in blanking dump bytes when dump address not specified and
    # this disables feature
    self.dumpaddr.trace('w', self.__lm32DumpAddrUpdated)

    # Bind a callback for changes to load/save state flags
    self.savestate.trace('w', self.__lm32SaveStateUpdated)
    self.loadstate.trace('w', self.__lm32SaveStateUpdated)
    
    # Convert CFG register default string to an integer
    cfg = int(self._LM32DEFAULTcfgword, 0)
    
    # Set the indivdual flag value to the bit masked value of the default CFG 
    # for their position (This makes combining easier later)
    self.mflag.set   (cfg & 0x00000001)
    self.dflag.set   (cfg & 0x00000002)
    self.sflag.set   (cfg & 0x00000004)
    self.xflag.set   (cfg & 0x00000010)
    self.ccflag.set  (cfg & 0x00000020)
    self.dcflag.set  (cfg & 0x00000040)
    self.icflag.set  (cfg & 0x00000080)
    self.gflag.set   (cfg & 0x00000100)
    self.hflag.set   (cfg & 0x00000200)
    self.rflag.set   (cfg & 0x00000400)
    self.jflag.set   (cfg & 0x00000800)
    
    self.cfgint.set ((cfg & 0x0003f000) >> 12)
    self.cfgbp.set  ((cfg & 0x003c0000) >> 18)
    self.cfgwp.set  ((cfg & 0x01c00000) >> 22)
    
    # If any of the CFG flag variables change (i.e. written to), call a function  
    self.mflag.trace ('w', self.__lm32CfgUpdated)
    self.dflag.trace ('w', self.__lm32CfgUpdated)
    self.sflag.trace ('w', self.__lm32CfgUpdated)
    self.xflag.trace ('w', self.__lm32CfgUpdated)
    self.ccflag.trace('w', self.__lm32CfgUpdated)
    self.dcflag.trace('w', self.__lm32CfgUpdated)
    self.icflag.trace('w', self.__lm32CfgUpdated)
    self.gflag.trace ('w', self.__lm32CfgUpdated)
    self.hflag.trace ('w', self.__lm32CfgUpdated)
    self.rflag.trace ('w', self.__lm32CfgUpdated)
    self.jflag.trace ('w', self.__lm32CfgUpdated)

    self.cfgint.trace ('w', self.__lm32CfgUpdated)
    self.cfgbp.trace  ('w', self.__lm32CfgUpdated)
    self.cfgwp.trace  ('w', self.__lm32CfgUpdated)

    self.fast_mode = BooleanVar()
    self.fast_mode.set(lm32gui._LM32DEFAULTfastmode)
    
  # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  # Define the 'Private' class methods
  # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  
  # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  # Callbacks
  # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++   
  
  # __lm32OpenFile()
  #
  # Callback from menu open file, to get ELF filename 
  #
  def __lm32OpenFile(self):

    # Open a file select dialog box
    fname = askopenfilename(filetypes = (('Elf files','*.elf'), ('all files','*.*')))
    
    # Only update the entry if the returned value not an empty string,
    # otherwise the checker fires an error when updating the box.
    if fname != '' :
      self.elfname.set(os.path.relpath(fname, self.rundir.get()))

  # __lm32OpenExecFolder()
  #
  # Callback from menu to select folder containing executable
  #
  def __lm32OpenExecFolder(self) :

    foldername = askdirectory()
    if foldername != '' :
      self.execfolder.set(os.path.relpath(foldername, self.rundir.get()))

  # __lm32OpenRunFolder()
  #
  # Callback from menu to select folder from which to run
  #
  def __lm32OpenRunFolder (self) :

    # Get the folder name via a dialog
    foldername = askdirectory()

    # If a folder was selected (rather than the widget 'cancelled')...
    if foldername != '':

      # The run folder is changing, so update the variables with relative paths
      self.elfname.set(os.path.relpath(os.path.realpath(self.elfname.get()), foldername))

      if self.execfolder.get() != lm32gui._LM32DEFAULTexecfolder :
        self.execfolder.set(os.path.relpath(os.path.realpath(self.execfolder.get()), foldername))

      if self.inifile.get() != lm32gui._LM32DEFAULTinifile :
        self.inifile.set(os.path.relpath(os.path.realpath(self.inifile.get()), foldername))

      if self.logfile.get() != lm32gui._LM32DEFAULTlogfile :
        self.logfile.set(os.path.relpath(os.path.realpath(self.logfile.get()), foldername))

      # Update the rundir state
      self.rundir.set(foldername)

      # Actually change directories
      os.chdir(foldername)

  # __lm32About()
  #
  # Callback from Help menu's About selection 
  #  
  @staticmethod
  def __lm32About():

    # Pop up a message box
    showinfo('About', '\nlm32.py—cpmico32 front end GUI\n\n'+
                      'Copyright © 2017 Wyvern Semiconductors Ltd.\nVersion 1.3\n' +
                      '\nContact: info@anita-simulators.org.uk' +
                      '\nwww.anita-simulators.org.uk/wyvernsemi')
  
  # __lm32GetIni()
  #
  # Callback for 'Browse' button of .ini file
  #  
  def __lm32GetIni(self) :
    # Open a file select dialog box
    fname = askopenfilename(filetypes = (('ini files','*.ini'), ('all files','*.*')))
    
    # Only update the entry if the returned value not an empty string,
    # otherwise the checker fires an error when updating the box.
    if fname != '' :
      self.inifile.set(os.path.relpath(fname, self.rundir.get()))

  # __lm32GetLog()
  #
  # Callback for 'Browse' button of .log file
  #
  def __lm32GetLog(self) :

    # Open a file select dialog box
    fname = askopenfilename(filetypes = (('Log files','*.log'), ('all files','*.*')))

    # Only update the entry if the returned value not an empty string,
    # otherwise the checker fires an error when updating the box.
    if fname != '' :
      self.logfile.set(os.path.relpath(fname, self.rundir.get()))

  # __lm32GetSav()
  #
  # Callback for 'Browse' button of .sav file
  #
  def __lm32GetSav(self) :

    # Open a file select dialog box
    fname = askopenfilename(filetypes = (('Save files','*.sav'), ('all files','*.*')))

    # Only update the entry if the returned value not an empty string,
    # otherwise the checker fires an error when updating the box.
    if fname != '' :
      self.savfilename.set(os.path.relpath(fname, self.rundir.get()))

  # __lm32SaveStateUpdated()
  #
  # Callback if save- or load-state flags change.
  #
  def __lm32SaveStateUpdated (self, object, lstidx, mode) :

    # If both load and save state flags clear, disable save file entry
    if self.savestate.get() == 0 and self.loadstate.get() == 0 :
      self.hdlsave.config(state = DISABLED)
    else :
      self.hdlsave.config(state = NORMAL)

  # __lm32DumpAddrUpdated()
  #
  # Callback if Dump Address variable has a change. Check dump address,
  # and if a valid value, enable the dump bytes widget
  #
  def __lm32DumpAddrUpdated(self, object, lstidx, mode) :
  
    dmpaddr = self.dumpaddr.get()
    if dmpaddr == lm32gui._LM32DEFAULTdumpaddr or dmpaddr == '' :
      self.dbhdl[0].config(state = DISABLED)
      self.dbhdl[1].config(state = DISABLED)
    else :
      self.dbhdl[0].config(state = NORMAL)
      self.dbhdl[1].config(state = NORMAL)

  # __lm32TcpUpdated()
  #
  # Callback if TCP gdb debug changed.
  #
  def __lm32TcpUpdated(self, object, lstidx, mode) :
    if self.enabletcp.get() == 0 :
      self.comport.set(self._LM32DEFAULTcomport)
    else :
      self.comport.set(self._LM32DEFAULTtcpport)

  # __lm32CfgUpdated()
  #
  # Callback for any update to CFG register variables
  #
  def  __lm32CfgUpdated(self, object, lstidx, mode) :

    cfg = int(self.cfgword.get(), 0) & 0xfe000000
    cfg |= self.mflag.get()
    cfg |= self.dflag.get()
    cfg |= self.sflag.get()
    cfg |= self.xflag.get()
    cfg |= self.ccflag.get()
    cfg |= self.dcflag.get()
    cfg |= self.icflag.get()
    cfg |= self.gflag.get()
    cfg |= self.hflag.get()
    cfg |= self.rflag.get()
    cfg |= self.jflag.get()

    rtnlist = [0]

    valstr = self.cfgint.get()
    if valstr != '' :
      if self._lm32CheckStringNum(valstr, 0, 32, rtnlist) == False :
        showerror('Error', 'Invalid CFG Interrupts setting')
        return
      else :
        cfg |= rtnlist[0]  <<  12

    valstr = self.cfgbp.get()
    if valstr != '' :
      if self._lm32CheckStringNum(valstr, 0, 4, rtnlist) == False :
        showerror('Error', 'Invalid CFG Breakpoints setting')
        return
      else :
        cfg |= rtnlist[0]  <<  18

    valstr = self.cfgwp.get()
    if valstr != '' :
      if self._lm32CheckStringNum(valstr, 0, 4, rtnlist) == False :
        showerror('Error', 'Invalid CFG Watchpoints setting')
        return
      else :
        cfg |= rtnlist[0]  <<  22

    cfgstr = format(cfg, '#010x')
    self.cfgword.set(cfgstr)

  # __lm32fastmodeUpdated()
  #
  # Callback for any update to fast_mode variable
  #
  def __lm32fastmodeUpdated(self, object, lstidx, mode) :

    # If the fast_mode variable is True, new state of fast mode exclusion list widgets
    # will be DISABLED, else it will be NORMAL
    if self.fast_mode.get() :
      newstate = DISABLED
    else :
      newstate = NORMAL

    # Flatten the lists from the two notebaook tables into a single list, as
    # all will be updated together
    flatlist = self.fastlist[0] + self.fastlist[1]

    # Set the new state for all the widgets in the exclusion list
    for hdl in flatlist :
       hdl.config(state = newstate)

    return
 
  # __lm32DebugCmd()
  #
  # Callback for 'Debug' button. Construct command string for cpu6502 and execute
  #
  def __lm32DebugCmd(self) :

    self.gdb.set(1)
    self.__lm32RunCmd()
    self.gdb.set(0)
        
  # __lm32RunCmd()
  #
  # Callback for 'Run' button. Construct command string for cpu6502 and execute
  #
  def __lm32RunCmd(self) :

    islnx  = False
    isFast = False

    # Flag if the lnxmico32 notebook tab is the active tab.
    if self.note.tab(self.note.select(), 'text') == 'lnxmico32' :
      islnx = True

    # Flag if the lnxmico32 notebook tab is the active tab.
    if self.fast_mode.get() :
      isFast = True

    # Get the configured folder
    folder = self.execfolder.get()

    # If the folder is the default setting, then set to the empty string
    if folder == lm32gui._LM32DEFAULTexecfolder :
      folder = ''

    # On windows (or Cygwin), run the .exe version
    if os.name == 'nt' or sys.platform == 'cygwin':
      if not islnx :
        execstr = 'cpumico32.exe '
      else :
        execstr = 'lnxmico32.exe '
    else :
      if not islnx :
        execstr = 'cpumico32 '
      else :
        execstr = 'lnxmico32 '

    cmdstr = os.path.join(folder, execstr)

    # Initialise the return list to be a single element list
    rtnlist = [0]
    
    # ------------------------------------------------
    # Flags
    
    flagstr = ''

    # TCP debugging enable (if not fast mode and gdb set)
    if not isFast :
      if self.enabletcp.get() != 0 and self.gdb.get() != 0 :
        flagstr += 't'
      # GDB debugging (only if -t not specified)
      elif self.gdb.get() != 0 :
        flagstr += 'g'

    # Dump regs
    if self.dumpregs.get() != 0 :
      flagstr += 'D'
      
    # Dump num instructions
    if self.dumpnuminstr.get() != 0 :
      flagstr += 'I'
      
    # Disassemble
    if not isFast and self.disassemble.get() != 0 :
      flagstr += 'x'

    # Verbose
    if not isFast and self.verbose.get() != 0 :
      flagstr += 'v'

    # Enable internal callbacks
    if not islnx and self.enablecb.get() != 0 :
      flagstr += 'T'

    # Disable breakpoint on lock
    if not isFast and self.disablebreakonlock.get() != 0 :
      flagstr += 'd'

    # For lnxmico32, set flags for save and restore
    if islnx :
      if self.savestate.get() != 0 :
        flagstr += 'S'

      if self.loadstate.get() != 0 :
        flagstr += 'L'

    # If the flag string isn't empty, add flags to cmdstr with a leading '-', and trailing space
    if flagstr != '' :
      cmdstr += '-' + flagstr + ' '    
    
    # ------------------------------------------------
    # Entry values

    # Memory entries only active when not lnxmico32
    if not islnx :

      # Memory size
      valstr = self.memsize.get()
      if self._lm32CheckStringNum(valstr, 1, 0xffffffff, rtnlist) != False :
        if valstr != lm32gui._LM32DEFAULTmemsize :
          cmdstr += '-m ' + valstr + ' '
      else :
        showerror('Error', 'Invalid mem size setting')
        return

      # Memory offset
      valstr = self.memoffset.get()
      if self._lm32CheckStringNum(valstr, 0, 0xffffffff, rtnlist) != False :
        if valstr != lm32gui._LM32DEFAULTmemoffset :
          cmdstr += '-o ' + valstr + ' '
      else :
        showerror('Error', 'Invalid mem offset setting')
        return

      # Memory entry point address
      valstr = self.memaddr.get()
      if self._lm32CheckStringNum(valstr, 0, 0xffffffff, rtnlist) != False :
        if valstr != lm32gui._LM32DEFAULTmemaddr :
          cmdstr += '-e ' + valstr + ' '
      else :
        showerror('Error', 'Invalid mem offset setting')
        return
      
    # Number of instructions (number or 'forever')
    if not isFast :
      valstr = self.numinstr.get()
      if self._lm32CheckStringNum(valstr, 1, 0xffffffff, rtnlist) != False :
        cmdstr += '-n ' + valstr + ' '
      else :
        if valstr != lm32gui._LM32DEFAULTnuminstr :
          showerror('Error', 'Invalid num instr setting')
          return
        
    # Dump address (number or 'NONE')
    valstr = self.dumpaddr.get()
    if self._lm32CheckStringNum(valstr, 0, 0xffffffff, rtnlist) != False :
      cmdstr += '-r ' + valstr + ' '
      
      # Only check number of dump bytes when dump address valid
      valstr = self.dumpbytes.get()
      if self._lm32CheckStringNum(valstr, 1, 0xffffffff, rtnlist) != False :
        if valstr != lm32gui._LM32DEFAULTdumpbytes :
          cmdstr += '-R ' + valstr + ' '
      else :
        showerror('Error', 'Invalid number of dump bytes')
        return
    else :
      if valstr != lm32gui._LM32DEFAULTdumpaddr :
        showerror('Error', 'Invalid dump address')
        return
        
    # CFG register values
    valstr = self.cfgword.get()
    if self._lm32CheckStringNum(valstr, 0, 0xffffffff, rtnlist) != False :
      if valstr != lm32gui._LM32DEFAULTcfgword :
        cmdstr += '-c ' + valstr + ' '
    else :
      showerror('Error', 'Invalid mem offset setting')
      return

    if not isFast :
      # Internal memory wait states
      valstr = self.waitstates.get()
      if self._lm32CheckStringNum(valstr, 0, 0xffffffff, rtnlist) != False :
        if valstr != lm32gui._LM32DEFAULTwaitstates :
          cmdstr += '-w ' + valstr + ' '
      else :
        showerror('Error', 'Invalid mem offset setting')
        return
    
      # Breakpoint address (number or 'NONE')
      valstr = self.bpaddr.get()
      if self._lm32CheckStringNum(valstr, 0, 0xffffffff, rtnlist) != False :
        cmdstr += '-b ' + valstr + ' '
      else :
        if valstr != lm32gui._LM32DEFAULTbpaddr :
          showerror('Error', 'Invalid breakpoint addr setting')
          return

      # Generate a port number argument. Only add the argument for TCP
      # enabled (-t) or -g specified on windows.
      valstr = self.comport.get()
      if self.enabletcp.get() != 0 or (os.name != 'nt' and self.gdb.get() != 0) :
        if self._lm32CheckStringNum(valstr, 0, 0xffff, rtnlist) != False :
          cmdstr += '-G ' + valstr + ' '
        else :
          showerror('Error', 'Invalid port number')
          return
        
    # -----------------------------------------------
    # Files

    # For lnxmico032, specify .sav filename
    if islnx :
      valstr = self.savfilename.get()
      if valstr != '' and valstr != lm32gui._LM32DEFAULTsavfilename :
        cmdstr += '-s ' + valstr + ' '
    
    # Configuration .ini file (filename or NONE)
    valstr = self.inifile.get()
    if valstr != '' and valstr != lm32gui._LM32DEFAULTinifile :
      cmdstr += '-i ' + valstr + ' '
      
    # Log file 
    valstr = self.logfile.get()
    if valstr != '' and valstr != lm32gui._LM32DEFAULTlogfile :
      cmdstr += '-l ' + valstr + ' '
      
    # ELF program file
    if not islnx :
      valstr = self.elfname.get()
      if valstr != '' and valstr != lm32gui._LM32DEFAULTelfname :
        cmdstr += '-f ' + valstr + ' '

    # Create a popup window for the output
    txtframe, txthdl = self.__lm32CreateStdoutPopup(islnx)

    # Print the command string that we're about to execute for future reference
    print (cmdstr)

    # Run the command in a new process
    self._lm32ShellCmd(cmdstr, self.rundir.get(), islnx)

    # If the log is not to stdout, append the file's contents to the text box
    if self.logfile.get() != 'stdout' :

      try :
        logfp = open(self.logfile.get())
        txthdl.insert(END, logfp.read())
        txthdl.see(END)
        logfp.close()
      except :
        showerror('Logfile Open', 'Error when reading logfile contents')

  # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  # Widget generators
  # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  # _lm32CreateWidgets()
  #  
  # Create the application GUI
  #
  # noinspection PyBroadException
  def __lm32CreateWidgets (self, top) :

    # Determine whether on windows or not, as this will make some differences
    if os.name == 'nt' :
      isWindows = True 
    else :
      isWindows = False
    
    # Create a menu bar and add to top widget
    menu = Menu(top)
    top.config(menu = menu)
    
    # Create a menu for finding the ELF file, and quitting application
    filemenu = Menu(master = menu, tearoff = 0)
    menu.add_cascade(label = 'File', menu = filemenu)
    filemenu.add_command   (label = 'Open Elf File...',     command = self.__lm32OpenFile)
    filemenu.add_command   (label = 'Change Run Folder...', command = self.__lm32OpenRunFolder)
    filemenu.add_command   (label = 'Executable Folder...',  command = self.__lm32OpenExecFolder)
    filemenu.add_separator ()
    filemenu.add_command   (label = 'Exit', command = top.quit)

    # Add a mde sub menu
    modemenu = Menu(master = menu, tearoff = 0)
    menu.add_cascade(label = 'Mode', menu = modemenu)
    modemenu.add_checkbutton (label = 'Fast Mode', onvalue = 1 , offvalue = 0, variable = self.fast_mode)
    self.fast_mode.trace('w', self.__lm32fastmodeUpdated)
    
    # Add a help menu, with an 'About' selection
    helpmenu = Menu(master = menu, tearoff = 0)
    menu.add_cascade(label = 'Help', menu=helpmenu)
    helpmenu.add_command(label = 'About...', command=self.__lm32About)

    self.note   = Notebook(master = top)
    tab1        = Frame(master = self.note)
    tab2        = Frame(master = self.note)

    self.note.add(child = tab1, text = 'cpumico32')
    self.note.add(child = tab2, text = 'lnxmico32')
    self.note.grid(row = 0, padx = 10, pady = 10)

    # Create a panel with the icon image. Image file expected to be in same location
    # as the script.
    imgerr = False
    img = ''
    try :
      img = PhotoImage(file = self.scriptdir.get() + '/' +'icon.gif')
    except :
      imgerr = True

    if not imgerr :
      panel1 = Label(master = tab1, image = img)
      panel2 = Label(master = tab2, image = img)
    else :
      # If the above threw an exception, just have an empty panel
      panel1 = Label(master = tab1, width = 15)
      panel2 = Label(master = tab2, width = 15)

    # Initialise lists as a two entry array here so they can be
    # referenced by the two calls below into different indexes
    self.dbhdl     = [None] * 2
    self.fastlist  = [None] * 2

    self.fastlist[0] = self.__lm32CreateSubWidgets(master = tab1, panel = panel1, isLnx = False)
    self.fastlist[1] = self.__lm32CreateSubWidgets(master = tab2, panel = panel2, isLnx = True)

    # Start of Tk event loop
    mainloop()

  # _lm32CreateSubWidgets()
  #
  # Create the application GUI's sub-widgets. Return a list of widget handles
  # that are not valid for fast mode.
  #
  # noinspection PyBroadException
  def __lm32CreateSubWidgets (self, master, panel, isLnx) :

    framerow    = 0
    totalcols   = 3
    tabidx      = 0
    fasthdllist = []

    if isLnx :
      tabidx =  1
    
    # Add check buttons in a frame, as column 0. Each tuple is (<label>, <Tk variable>, <onvalue>).
    # Two dimensional array is tupleList[rows][cols]
    flagsframe = LabelFrame(master = master, text = 'Flags:', padding = 20)
    tupleList = [
      [('Dump registers',     self.dumpregs,    1), ('Dump # instructions',   self.dumpnuminstr,       1)],
      [('Disassemble',        self.disassemble, 1), ('Verbose',               self.verbose,            1)],
      [('TCP gdb connection', self.enabletcp,   1), ('Disable Break on Lock', self.disablebreakonlock, 1)]
    ]

    # Append cpumico32/lnxmico32 specific flags as appropriate
    if not isLnx :
      tupleList.append([('Enable Internal callbacks', self.enablecb,    1)])
    else :
      tupleList.append([('Save state on Exit', self.savestate, 1), ('Load state at start', self.loadstate, 1)])

    hdls = self.addCheckButtonRows('', tupleList, flagsframe)
    flagsframe.grid(row = framerow, padx = 10, pady = 10, sticky = W)

    # Add to fast mode exclusion list handles from 'Disassemble' to 'Disable Break on Lock
    # (indexs 2 to 5)
    fasthdllist.extend(hdls[2:6])

    # Add a trace on the TCP enable variable
    self.enabletcp.trace('w', self.__lm32TcpUpdated)

    panel.grid(row = framerow, column = 2, pady = 10, padx = 5)
    
    # Add Entry widgets in a new frame.  Each tuple is (<label>, <Tk variable>, <width>).
    # Two dimensional array is tupleList[rows][cols]
    entryframe = LabelFrame(master = master, text = 'Variables:', padding = 17)
    tupleList = [   
      [('CFG register',  self.cfgword,   10),('Dump Addr',    self.dumpaddr,  10),('# Dump Bytes',    self.dumpbytes,10)]
    ]

    if not isLnx :
      tupleList.append(
        [('Mem size',      self.memsize,   10),('Mem offset',   self.memoffset, 10),('Mem Entry Addr', self.memaddr,  10)]
      )

    tupleList.extend((
        [('Int mem waits', self.waitstates,10),('# Run Instr\'s',self.numinstr, 10),('Break point addr',self.bpaddr, 10)],
        [('Debug port #',  self.comport,   10)]
      ))

    hdls = self.addEntryRows('', tupleList, entryframe)

    # Add to fast mode exclusion list widget handles from the last 4 entries
    # ('Int mem waits' to 'Debug port #')
    fasthdllist.extend(hdls[-4:])
    
    # Extract handles of dump bytes from returned handle list
    self.dbhdl[tabidx]  = hdls[2]
    
    # The default dump address (of NONE) makes dump bytes moot, so disable
    self.dbhdl[tabidx].config(state = DISABLED)
    
    # Disable the CFG register entry box (hdls index 0), and bind
    hdls[0].config(state = DISABLED)
    hdls[0].bind('<Double-Button-1>', self.__lm32CreateCfgPopup)
    
    # Add Entry widget frame in a new row, spanning all three columns
    framerow +=1
    entryframe.grid(row = framerow, columnspan = totalcols, padx = 10, pady = 10, sticky = W+E)
    
    # Add utility files selection in a new frame. Each tuple is
    # (<label>, <Tk variable>, <width>, <button label>, <callback>).
    # Two dimensional array is tupleList[rows][cols]
    fileframe = LabelFrame(master = master, text='Files:', padding = 5)
    tupleList = [[('.ini file',     self.inifile,    50, 'Browse', self.__lm32GetIni)],
                 [('Log file',      self.logfile,    50, 'Browse', self.__lm32GetLog)]
                ]
    # For lnxmico32, add a browse button for .sav file
    if isLnx :
      tupleList.append([('Save state file', self.savfilename,  50, 'Browse', self.__lm32GetSav)])

    # If not lnxmico32, append the program file entry data to the list
    if not isLnx :
      self.__none = None
      tupleList.append([('Program file', self.elfname, 50, '', self.__none)])

    hdls = self.addFileEntryRows('', tupleList, fileframe)

    if not isLnx :
      # If not lnxmico32, disable the program file entry
      hdls[4].config(state = DISABLED)
    else :
      # If lnxmico32 make available to callbacks the save file entry handle,
      # and call the flag trace callback __lm32SaveStateUpdated() to set the
      # correct disable/normal state.
      self.hdlsave = hdls[4]
      self.__lm32SaveStateUpdated(None, None, None)
    
    # Add utility file widgets frame in a new row, spanning all three columns
    framerow +=1
    fileframe.grid(row = framerow, columnspan = totalcols, padx = 10, pady = 10, sticky = W+E)

    # Add directory selection in a new frame. Each tuple is
    # (<label>, <Tk variable>, <width>, <button label>, <callback>).
    # Two dimensional array is tupleList[rows][cols]
    dirframe = LabelFrame(master = master, text = 'Directories:', padding = 5)
    tupleList = [[('executable Dir', self.execfolder, 50, '', None)],
                 [('Run Dir',        self.rundir,     50, '', None)]
                ]
    hdls = self.addFileEntryRows('', tupleList, dirframe)

    hdls[0].config(state = DISABLED)
    hdls[1].config(state = DISABLED)

    # Add utility file widgets frame in a new row, spanning all three columns
    framerow += 1
    dirframe.grid(row = framerow, columnspan = totalcols, padx = 10, pady = 10, sticky = W+E)
    
    # Add a 'Run' button to a new frame
    execframe = Frame(master = master,  padding = 5)
    Button(master = execframe, text = 'Run',   command = self.__lm32RunCmd).grid(row = 0, column = 0, padx=10)

    # Add a 'Debug' button
    dbghdl = Button(master = execframe, text = 'Debug', command = self.__lm32DebugCmd)
    dbghdl.grid (row = 0, column = 1, padx=10)

    # Add the Debug button widget handle to the fast mode exclusion list
    fasthdllist.append(dbghdl)
    
    # Add Run/Debug button frame in a new row, spanning all three columns
    framerow +=1
    execframe.grid(row = framerow, columnspan = totalcols, padx=10, pady = 10)

    # Return the fast mode exclusion list
    return fasthdllist
  
  # __lm32CreateCfgPopup()
  #
  # Create the CFG register pop up
  #  
  def __lm32CreateCfgPopup (self, event) :
  
    toplevel = Toplevel()
    toplevel.title('CFG Register')

    # Change the default icon to something more appropriate
    self.__lm32SetIcon(toplevel)
    
    # Create a notebook (tabs), generate two frames for it and
    # add each as a tab
    note = Notebook(master = toplevel)
    tab1 = Frame(master = note)
    tab2 = Frame(master = note)

    note.add(child = tab1, text = 'Flags')
    note.add(child = tab2, text = 'Values')
    
    curr_row  = 0
    
    # Add the notebook as the first row
    note.grid(row = curr_row, padx = 10, pady = 10)
    curr_row += 1
    
    # Set check button configuration. Each tuple is (<label>, <Tk variable>, <onvalue>).
    # Two dimensional array is tupleList[rows][cols]
    tupleList = [
      [('Multiply',    self.mflag,  0x001), ('Divide',      self.dflag,  0x002), ('Barrel',    self.sflag,  0x004)],
      [('Sign Ext',    self.xflag,  0x010), ('Cycle count', self.ccflag, 0x020), ('JTAG',      self.jflag,  0x800)],
      [('SW Debug',    self.gflag,  0x100), ('HW Debug',    self.hflag,  0x200), ('ROM debug', self.rflag,  0x400)],
      [('Instr Cache', self.icflag, 0x080), ('Data Cache',  self.dcflag, 0x040)]
    ]

    # Generate the check button widgets in the cfgflagsframe frame (first notebook tab)            
    cfgflagsframe = Frame(master = tab1)
    cfgflagsframe.grid(padx = 10)
    hdllist = self.addCheckButtonRows(' ', tupleList, cfgflagsframe)
    
    # Set CFG entry configuration. Each tuple is (<label>, <Tk variable>, <width>).
    # Two dimensional array is tupleList[rows][cols]
    tupleList = [[('Number of External Interrupts (0 to 32)', self.cfgint, 5)],
                 [('Number of Breakpoint CSRs (0 to 4)',      self.cfgbp,  5)],
                 [('Number of Watchpoint CSRs (0 to 4)',      self.cfgwp,  5)]]
                 
    # Generate the entry widgets in the cfgentryframe frame (second notebook tab)
    cfgentryframe = Frame(master = tab2)
    cfgentryframe.grid(padx = 10)
    hdls = self.addEntryRows(' ', tupleList, cfgentryframe)
    
    # In the second row, add a close button
    btn = Button(master = toplevel, text = 'Close', command = toplevel.destroy)
    btn.grid (row = curr_row, column = 0, pady = 10)
    curr_row += 1

  # __lm32CreateStdoutPopup()
  #
  # Create the cpumico32 run output pop up
  #
  def __lm32CreateStdoutPopup (self, islnx) :

    curr_row = 0

    # Create a top level window
    toplevel = Toplevel()
    toplevel.title('output')

    # Change the default icon to something more appropriate
    self.__lm32SetIcon(toplevel)

    # Create a scroll bar widget
    scrollbar = Scrollbar(master = toplevel)

    # Add a text box to first row
    txtbox = Text(master = toplevel, height = 40, width = 80, yscrollcommand = scrollbar.set)
    txtbox.grid(row = curr_row, column = 0, pady = 5, padx = 5)

    # Add the scroll bar in the same row as the text box, and connect to its Y view
    scrollbar.grid(row = curr_row, column = 1, stick = N+S)
    scrollbar.config(command  = txtbox.yview)

    # Redirect stdout to the textbox using the previously defined class
    # to replace the sys one.
    sys.stdout = lm32Stdout(txtbox)
    curr_row += 1

    # In the next row, add a close button
    btn = Button(master = toplevel, text = 'Close', command = toplevel.destroy)
    btn.grid(row = curr_row, column = 0, pady = 10)

    if islnx :
      toplevel.wm_state('iconic')

    return toplevel, txtbox


  # _lm32SetIcon()
  #
  # Set the icon for the given widget (windows only)
  #
  def __lm32SetIcon(self, widget) :

    if os.name == 'nt' :
      widget.iconbitmap(os.path.join(self.scriptdir.get(), 'favicon.ico'))

  # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  # 'Public' methods
  # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
  
  # run()
  #
  # Top level method to create application window, and generate output
  #  
  def run(self):
  
    # Create the application GUI
    self.__lm32CreateWidgets(self.root)
    
  
# ###############################################################
# Only run if not imported
#
if __name__ == '__main__' :
  
  gui = lm32gui()
  gui.run()

