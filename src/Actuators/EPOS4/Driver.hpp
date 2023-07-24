//***************************************************************************
// Author: Renato Campos                                                    *
//***************************************************************************

#ifndef ACTUATORS_EPOS4_DRIVER_HPP_INCLUDED_
#define ACTUATORS_EPOS4_DRIVER_HPP_INCLUDED_

#include <DUNE/DUNE.hpp>

// Local Headers
#include "Definitions.h"

namespace Actuators {
namespace EPOS4 {
using DUNE_NAMESPACES;

enum ResultEnum { MMC_SUCCESS = 0, MMC_FAILED = 1 };

enum OpMode {
  NONE,
  PVM = 3,  // Profile Velocity Moe
  CST = 10, // Cyclic Synchronous Current Mode
};

struct Version {
  uint16_t hardware;
  uint16_t software;
  uint16_t app_number;
  uint16_t app;
};

class Driver {
private:
  void *m_key_handle = 0;

  OpMode m_op_mode;
  std::string m_device_name = "EPOS4";
  std::string m_protocol_stack = "MAXON SERIAL V2";
  std::string m_interface_name = "USB";
  std::string m_port_name = "USB0";
  unsigned short m_node_id = 1;
  unsigned int m_baudrate = 1000000;

public:
  Driver() : m_op_mode(OpMode::NONE) { setDefaultParameters(); }

  ~Driver() {
    if (m_key_handle != 0) {
      closeDevice();
    }
    m_key_handle = 0;
  }

  void setDefaultParameters() {
    // USB
    m_node_id = 1;
    m_device_name = "EPOS4";
    m_protocol_stack = "MAXON SERIAL V2";
    m_interface_name = "USB";
    m_port_name = "USB0";
    m_baudrate = 1000000;
  }

  std::string getErrorString(unsigned int &error_code) {
    char error_info[255];
    VCS_GetErrorInfo(error_code, error_info, 254);
    return std::string(error_info);
  }

  ResultEnum openDevice(unsigned int &error_code) {
    ResultEnum result = MMC_FAILED;

    m_key_handle = VCS_OpenDevice((char *)m_device_name.c_str(),
                                  (char *)m_protocol_stack.c_str(),
                                  (char *)m_interface_name.c_str(),
                                  (char *)m_port_name.c_str(), &error_code);

    if (m_key_handle != 0 && error_code == 0) {
      // m_parent->spew("Device opened");
      unsigned int l_baudrate = 0;
      unsigned int l_timeout = 0;

      if (VCS_GetProtocolStackSettings(m_key_handle, &l_baudrate, &l_timeout,
                                       &error_code) != 0) {
        if (VCS_SetProtocolStackSettings(m_key_handle, m_baudrate, l_timeout,
                                         &error_code) != 0) {
          if (VCS_GetProtocolStackSettings(m_key_handle, &l_baudrate,
                                           &l_timeout, &error_code) != 0) {
            if (m_baudrate == l_baudrate) {
              result = MMC_SUCCESS;
            }
          }
        }
      }
    } else {
      // m_parent->spew("Failed to open device");
      m_key_handle = 0;
      result = MMC_FAILED;
    }

    return result;
  }

  ResultEnum getAvailableInterfaces(std::vector<std::string> &itfs) {
    ResultEnum result = MMC_FAILED;
    int lStartOfSelection = 1;
    int lMaxStrSize = 255;
    char *pInterfaceNameSel = new char[lMaxStrSize];
    int lEndOfSelection = 0;
    unsigned int ulErrorCode = 0;
    itfs.clear();
    do {
      if (!VCS_GetInterfaceNameSelection(
              (char *)m_device_name.c_str(), (char *)m_protocol_stack.c_str(),
              lStartOfSelection, pInterfaceNameSel, lMaxStrSize,
              &lEndOfSelection, &ulErrorCode)) {
        result = MMC_FAILED;
        break;
      } else {
        result = MMC_SUCCESS;
        itfs.push_back(std::string(pInterfaceNameSel));
      }

      lStartOfSelection = 0;
    } while (lEndOfSelection == 0);

    delete[] pInterfaceNameSel;

    return result;
  }

  ResultEnum getAvailablePorts(const char *p_pInterfaceNameSel,
                               std::vector<std::string> &ports) {
    ResultEnum result = MMC_FAILED;
    int lStartOfSelection = 1;
    int lMaxStrSize = 255;
    char *pPortNameSel = new char[lMaxStrSize];
    int lEndOfSelection = 0;
    unsigned int ulErrorCode = 0;

    do {
      if (!VCS_GetPortNameSelection(
              (char *)m_device_name.c_str(), (char *)m_protocol_stack.c_str(),
              (char *)p_pInterfaceNameSel, lStartOfSelection, pPortNameSel,
              lMaxStrSize, &lEndOfSelection, &ulErrorCode)) {
        result = MMC_FAILED;
        break;
      } else {
        result = MMC_SUCCESS;
        ports.push_back(std::string(pPortNameSel));
        ;
      }

      lStartOfSelection = 0;
    } while (lEndOfSelection == 0);

    return result;
  }

  ResultEnum closeDevice() {

    ResultEnum result = MMC_FAILED;
    unsigned int error_code = 0;
    if (VCS_CloseDevice(m_key_handle, &error_code) != 0 && error_code == 0) {
      result = MMC_SUCCESS;
      // m_parent->inf("Closed device");
    }
    return result;
  }

  ResultEnum setCSTMode() {
    ResultEnum result = MMC_SUCCESS;
    unsigned int error_code;
    if (VCS_SetOperationMode(m_key_handle, m_node_id, (char)OpMode::CST,
                             &error_code) == 0) {
      result = MMC_FAILED;
      // m_parent->err("Failed to set CST mode - error code %u", error_code);
    } else {
      m_op_mode = OpMode::CST;
    }
    return result;
  }

  ResultEnum setProfileVelocityMode() {
    ResultEnum result = MMC_SUCCESS;
    unsigned int error_code;
    if (VCS_ActivateProfileVelocityMode(m_key_handle, m_node_id, &error_code) ==
        0) {
      result = MMC_FAILED;
      // m_parent->err("Failed to activate profile velocity mode - error code:
      // %u", error_code);
    } else {
      m_op_mode = OpMode::PVM;
    }
    return result;
  }

  ResultEnum getFaultState(bool &is_fault) {
    int fault;
    unsigned int error_code = 0;
    ResultEnum result = MMC_SUCCESS;
    if (VCS_GetFaultState(m_key_handle, m_node_id, &fault, &error_code) == 0) {
      // m_parent->err("Failed to get fault state - error code: %u",
      // error_code);
      result = MMC_FAILED;
    } else {
      is_fault = fault == 1 ? true : false;
    }
    return result;
  }

  ResultEnum clearFaults() {
    bool is_fault;
    unsigned int error_code = 0;
    ResultEnum result = MMC_SUCCESS;
    if (getFaultState(is_fault) == MMC_FAILED) {
      result = MMC_FAILED;
    } else {
      if (is_fault) {
        if (VCS_ClearFault(m_key_handle, m_node_id, &error_code) == 0) {
          // m_parent->err("Failed to clear fault - error code: %u",
          // error_code);
          result = MMC_FAILED;
        } else {
          // m_parent->inf("Faults cleared");
        }
      }
    }
    return result;
  }

  ResultEnum enable() {
    if (clearFaults() == MMC_FAILED)
      return MMC_FAILED;
    int is_enabled = 0;
    unsigned int error_code = 0;
    ResultEnum result = MMC_SUCCESS;

    if (VCS_GetEnableState(m_key_handle, m_node_id, &is_enabled, &error_code) ==
        0) {
      // m_parent->err("Failed to get enabled state - error code: %u",
      // error_code);
      result = MMC_FAILED;
    } else {
      if (!is_enabled) {
        if (VCS_SetEnableState(m_key_handle, m_node_id, &error_code) == 0) {
          // m_parent->err("Failed to enable state - error code: %u",
          // error_code);
          result = MMC_FAILED;
        }
      }
    }
    return result;
  }

  ResultEnum setTargetVelocity(long target_velocity_rpm) {
    unsigned int error_code = 0;
    ResultEnum result = MMC_SUCCESS;
    if (m_op_mode != OpMode::PVM) {
      if (setProfileVelocityMode() == MMC_FAILED) {
        return MMC_FAILED;
      }
    }
    if (VCS_MoveWithVelocity(m_key_handle, m_node_id, target_velocity_rpm,
                             &error_code) == 0) {
      result = MMC_FAILED;
      // m_parent->err("VCS_MoveWithVelocity - error code: %u", error_code);
    }
    return result;
  }

  ResultEnum setTargetTorque(long target_torque_percentage) {
    unsigned int error_code = 0;
    ResultEnum result = MMC_SUCCESS;
    if (m_op_mode != OpMode::CST) {
      if (setCSTMode() == MMC_FAILED) {
        return MMC_FAILED;
      }
    }
    if (VCS_SetCurrentMustEx(m_key_handle, m_node_id, target_torque_percentage,
                             &error_code) == 0) {
      result = MMC_FAILED;
      // m_parent->err("Failed to set desired current - error code %u",
      // error_code);
    }
    return result;
  }

  ResultEnum getMaxVelocityRpm(unsigned int &max_velocity_rpm) {
    unsigned int error_code = 0;
    if (VCS_GetMaxProfileVelocity(m_key_handle, m_node_id, &max_velocity_rpm,
                                  &error_code) == 0) {
      // m_parent->err("Failed to get max profile velocity - error code %u",
      // error_code);
      return ResultEnum::MMC_FAILED;
    }
    return ResultEnum::MMC_SUCCESS;
  }

  ResultEnum haltVelocityMovement() {
    unsigned int error_code = 0;
    ResultEnum result = MMC_SUCCESS;
    if (VCS_HaltVelocityMovement(m_key_handle, m_node_id, &error_code) == 0) {
      result = MMC_FAILED;
      // m_parent->err("VCS_HaltVelocityMovement - error code: %u", error_code);
    } else {
      m_op_mode = OpMode::NONE;
    }
    return result;
  }

  ResultEnum getRpm(int &rpm) {
    unsigned int error_code = 0;
    ResultEnum result = MMC_SUCCESS;

    if (VCS_GetVelocityIsAveraged(m_key_handle, m_node_id, &rpm, &error_code) ==
        0) {
      result = MMC_FAILED;
      // m_parent->err("Failed to get velocity reading - error code %u",
      // error_code);
    }
    return result;
  }

  ResultEnum getDeviceVersion(Version &version, unsigned int &error_code) {
    ResultEnum result = MMC_FAILED;

    if (VCS_GetVersion(m_key_handle, m_node_id, &version.hardware,
                       &version.software, &version.app_number, &version.app,
                       &error_code)) {
      result = MMC_SUCCESS;
    }
    return result;
  }
};

} // namespace EPOS4
} // namespace Actuators

#endif