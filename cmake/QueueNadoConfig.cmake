#.rst:
# FindQueueNado
# -------
#
# Find libQueueNado
# QueueNado deals with file access and directory traversal.
##
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``QueueNado_INCLUDE_DIRS``
#   where to find 
# Alien.h      CZMQToolkit.h    Headcrab.h         Kraken.h    
# QueueNadoMacros.h     Rifle.h            Shotgun.h    ZeroMQ.h
# BoomStick.h  HarpoonBattle.h  IComponentQueue.h  Listener.h  
# ReceiveDpiMsgLRZMQ.h  SendDpiMsgLRZMQ.h  Skelleton.h
# Crowbar.h    Harpoon.h        KrakenBattle.h     Notifier.h  
# ReceivePacketZMQ.h    SendPacketZMQ.h    Vampire.h


#
# ``QueueNado_LIBRARIES``
#   the libraries to link against to use libQueueNado.
#   that includes libQueueNado library files.
#
# ``QueueNado_FOUND``
#   If false, do not try to use QueueNado.
#
include(FindPackageHandleStandardArgs)
find_path(QueueNado_INCLUDE_DIR Alien.h CZMQToolkit.h Headcrab.h Kraken.h QueueNadoMacros.h  Rifle.h Shotgun.h ZeroMQ.h BoomStick.h  HarpoonBattle.h IComponentQueue.h Listener.h ReceiveDpiMsgLRZMQ.h  SendDpiMsgLRZMQ.h Skelleton.h Crowbar.h  Harpoon.h KrakenBattle.h Notifier.h ReceivePacketZMQ.h SendPacketZMQ.h Vampire.h
)
find_library(QueueNado_LIBRARY
            NAMES libQueueNado QueueNado)

find_package_handle_standard_args(QueueNado  DEFAULT_MSG
            QueueNado_INCLUDE_DIR QueueNado_LIBRARY)
            
mark_as_advanced(QueueNado_INCLUDE_DIR QueueNado_LIBRARY)
set(QueueNado_LIBRARIES ${QueueNado_LIBRARY})
set(QueueNado_INCLUDE_DIRS ${QueueNado_INCLUDE_DIR})