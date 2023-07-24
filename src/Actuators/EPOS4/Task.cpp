//***************************************************************************
// Author: RC                                                               *
//***************************************************************************

#include <memory>

// DUNE headers.
#include <DUNE/DUNE.hpp>

// Local headers
#include "Driver.hpp"
namespace Actuators {
namespace EPOS4 {
using DUNE_NAMESPACES;

struct Task : public DUNE::Tasks::Task {
  std::unique_ptr<Driver> m_driver;

  //! Constructor.
  //! @param[in] name task name.
  //! @param[in] ctx context.
  Task(const std::string &name, Tasks::Context &ctx)
      : DUNE::Tasks::Task(name, ctx) {}

  //! Update internal state with new parameter values.
  void onUpdateParameters() override {}

  //! Reserve entity identifiers.
  void onEntityReservation() override {}

  //! Resolve entity names.
  void onEntityResolution() override {}

  //! Acquire resources.
  void onResourceAcquisition() override {
    uint32_t error_code = 0;
    debug("Running on resource acquisition");
    m_driver = std::make_unique<Driver>();

    std::vector<std::string> itfs;
    debug("Getting available interfaces");

    if (m_driver->getAvailableInterfaces(itfs)) {
      for (const auto &itf : itfs) {
        debug("interface %s", itf.c_str());
      }
      if (m_driver->openDevice(error_code)) {
        std::string error_string = m_driver->getErrorString(error_code);
        debug("failed to open device: %s", error_string.c_str());
      }
    } else {
      throw std::runtime_error("ESC not connected");
    }
  }

  //! Initialize resources.
  void onResourceInitialization() override {}

  //! Release resources.
  void onResourceRelease() override {
    debug("Running on resource release");
    m_driver.reset();
    debug("Resource release is complete");
  }

  //! Main loop.
  void onMain() override {
    while (!stopping()) {
      waitForMessages(1.0);
    }
  }
};
} // namespace EPOS4
} // namespace Actuators

DUNE_TASK
