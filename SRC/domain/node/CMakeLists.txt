#==============================================================================
# 
#        OpenSees -- Open System For Earthquake Engineering Simulation
#                Pacific Earthquake Engineering Research Center
#
#==============================================================================
target_sources(OPS_Domain
  PRIVATE
    Node.cpp
#   HeapNode.cpp
    NodalLoad.cpp
  PUBLIC
    Node.h
#   HeapNode.h
    NodalLoad.h
)

target_include_directories(OPS_Domain PUBLIC ${CMAKE_CURRENT_LIST_DIR})
