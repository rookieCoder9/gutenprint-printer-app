#include <pappl/pappl.h>
#include <gutenprint/gutenprint.h>

pappl_pr_driver_t get_drivers()
{
    const stp_printer_t *printer;
    int driver_count = stp_printer_model_count();
    pappl_pr_driver_t drivers_list[driver_count];
    for (int i = 0; i < driver_count; i++)
    {
        if ((printer = stp_get_printer_by_index(i)) != NULL)
        {
            const char *device_id;
            if (!strcmp(stp_printer_get_family(printer), "ps") ||
                !strcmp(stp_printer_get_family(printer), "raw"))
                continue;

            device_id = stp_printer_get_device_id(printer);
        }
    }
}