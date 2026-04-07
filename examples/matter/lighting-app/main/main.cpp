/*
 * RT582 Matter Lighting App — Entry point.
 *
 * Starts the AppTask which initialises the Matter stack and enters the
 * main event loop.
 */

#include "AppTask.h"

#include <system/SystemError.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, CONFIG_CHIP_APP_LOG_LEVEL);

int main()
{
    CHIP_ERROR err = AppTask::Instance().StartApp();

    LOG_ERR("Exited with code %" CHIP_ERROR_FORMAT, err.Format());
    return err == CHIP_NO_ERROR ? EXIT_SUCCESS : EXIT_FAILURE;
}
