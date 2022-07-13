//
// HP-Printer app for the Printer Application Framework
//
// Copyright © 2020-2022 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

//
// Include necessary headers...
//

#include <pappl/pappl.h>
#include <gutenprint/gutenprint.h>
#include "icons.h"
#include <math.h>

//
// PCL 6/PCL-XL support is current experimental and I've run into enough
// printers that can't handle the current code that I'm uncomfortable enabling
// it by default.  Define "WITH_PCL6" to enable the code, otherwise only the
// PCL 3/5 drivers are enabled.
//

//#define WITH_PCL6

//
// CUPS v3 makes some API changes.  These defines allow the same code to work
// for the v2 and v3 APIs...
//

#if CUPS_VERSION_MAJOR < 3
#define cups_page_header_t cups_page_header2_t
#endif // CUPS_VERSION_MAJOR < 3

//
// Constants...
//

typedef enum hp_driver_e // Drivers
{
  HP_DRIVER_DESKJET, // PCL 3 Deskjet
  HP_DRIVER_GENERIC, // PCL 5 generic
#if WITH_PCL6
  HP_DRIVER_GENERIC6,  // PCL 6 generic B&W
  HP_DRIVER_GENERIC6C, // PCL 6 generic color
#endif                 // WITH_PCL6
  HP_DRIVER_LASERJET   // PCL 5 LaserJet
} hp_driver_t;

#if WITH_PCL6
enum pcl6_attr
{
  PCL6_ATTR_COLOR_SPACE = 3,
  PCL6_ATTR_MEDIA_SIZE = 37,
  PCL6_ATTR_MEDIA_SOURCE = 38,
  PCL6_ATTR_MEDIA_TYPE = 39,
  PCL6_ATTR_ORIENTATION = 40,
  PCL6_ATTR_SIMPLEX_PAGE_MODE = 52,
  PCL6_ATTR_DUPLEX_PAGE_MODE = 53,
  PCL6_ATTR_DUPLEX_PAGE_SIDE = 54,
  PCL6_ATTR_POINT = 76,
  PCL6_ATTR_COLOR_DEPTH = 98,
  PCL6_ATTR_BLOCK_HEIGHT = 99,
  PCL6_ATTR_COLOR_MAPPING = 100,
  PCL6_ATTR_COMPRESS_MODE = 101,
  PCL6_ATTR_DESTINATION_BOX = 102,
  PCL6_ATTR_DESTINATION_SIZE = 103,
  PCL6_ATTR_SOURCE_HEIGHT = 107,
  PCL6_ATTR_SOURCE_WIDTH = 108,
  PCL6_ATTR_START_LINE = 109,
  PCL6_ATTR_PAD_BYTES_MULTIPLE = 110,
  PCL6_ATTR_BLOCK_BYTE_LENGTH = 111,
  PCL6_ATTR_DATA_ORG = 130,
  PCL6_ATTR_MEASURE = 134,
  PCL6_ATTR_SOURCE_TYPE = 136,
  PCL6_ATTR_UNITS_PER_MEASURE = 137,
  PCL6_ATTR_ERROR_REPORT = 143
};

enum pcl6_cmd
{
  PCL6_CMD_BEGIN_SESSION = 0x41,
  PCL6_CMD_END_SESSION = 0x42,
  PCL6_CMD_BEGIN_PAGE = 0x43,
  PCL6_CMD_END_PAGE = 0x44,
  PCL6_CMD_OPEN_DATA_SOURCE = 0x48,
  PCL6_CMD_CLOSE_DATA_SOURCE = 0x49,
  PCL6_CMD_SET_COLOR_SPACE = 0x6a,
  PCL6_CMD_SET_CURSOR = 0x6b,
  PCL6_CMD_BEGIN_IMAGE = 0xb0,
  PCL6_CMD_READ_IMAGE = 0xb1,
  PCL6_CMD_END_IMAGE = 0xb2
};

enum pcl6_color_depth
{
  PCL6_E_1_BIT,
  PCL6_E_4_BIT,
  PCL6_E_8_BIT
};

enum pcl6_color_mapping
{
  PCL6_E_DIRECT_PIXEL,
  PCL6_E_INDEXED_PIXEL,
  PCL6_E_DIRECT_PLANE
};

enum pcl6_color_space
{
  PCL6_E_BI_LEVEL,
  PCL6_E_GRAY,
  PCL6_E_RGB,
  PCL6_E_CMY,
  PCL6_E_CIELAB,
  PCL6_E_CRGB,
  PCL6_E_SRGB
};

enum pcl6_compress_mode
{
  PCL6_E_NO_COMPRESSION,
  PCL6_E_RLE_COMPRESSION,
  PCL6_E_JPEG_COMPRESSION,
  PCL6_E_DELTA_ROW_COMPRESSION
};

enum pcl6_data_org
{
  PCL6_E_BINARY_HIGH_BYTE_FIRST,
  PCL6_E_BINARY_LOW_BYTE_FIRST
};

enum pcl6_data_source
{
  PCL6_E_DEFAULT
};

enum pcl6_data_type
{
  PCL6_E_UBYTE,
  PCL6_E_SBYTE,
  PCL6_E_UINT16,
  PCL6_E_SINT16
};

enum pcl6_duplex_page_mode
{
  PCL6_E_DUPLEX_HORIZONTAL_BINDING, // Long-edge
  PCL6_E_DUPLEX_VERTICAL_BINDING    // Short-edge
};

enum pcl6_duplex_page_side
{
  PCL6_E_FRONT_MEDIA_SIDE,
  PCL6_E_BACK_MEDIA_SIDE
};

enum pcl6_enc
{
  PCL6_ENC_UBYTE = 0xc0,
  PCL6_ENC_UINT16 = 0xc1,
  PCL6_ENC_UINT32 = 0xc2,
  PCL6_ENC_SINT16 = 0xc3,
  PCL6_ENC_SINT32 = 0xc4,
  PCL6_ENC_REAL32 = 0xc5,

  PCL6_ENC_UBYTE_ARRAY = 0xc8,
  PCL6_ENC_UINT16_ARRAY = 0xc9,
  PCL6_ENC_UINT32_ARRAY = 0xca,
  PCL6_ENC_SINT16_ARRAY = 0xcb,
  PCL6_ENC_SINT32_ARRAY = 0xcc,
  PCL6_ENC_REAL32_ARRAY = 0xcd,

  PCL6_ENC_UBYTE_XY = 0xd0,
  PCL6_ENC_UINT16_XY = 0xd1,
  PCL6_ENC_UINT32_XY = 0xd2,
  PCL6_ENC_SINT16_XY = 0xd3,
  PCL6_ENC_SINT32_XY = 0xd4,
  PCL6_ENC_REAL32_XY = 0xd5,

  PCL6_ENC_UBYTE_BOX = 0xe0,
  PCL6_ENC_UINT16_BOX = 0xe1,
  PCL6_ENC_UINT32_BOX = 0xe2,
  PCL6_ENC_SINT16_BOX = 0xe3,
  PCL6_ENC_SINT32_BOX = 0xe4,
  PCL6_ENC_REAL32_BOX = 0xe5,

  PCL6_ENC_ATTR_UBYTE = 0xf8,
  PCL6_ENC_ATTR_UINT16 = 0xf9,
  PCL6_ENC_EMBEDDED_DATA = 0xfa,
  PCL6_ENC_EMBEDDED_DATA_BYTE = 0xfb
};

enum pcl6_error_report
{
  PCL6_E_NO_REPORTING,
  PCL6_E_ERROR_PAGE = 2
};

enum pcl6_measure
{
  PCL6_E_INCH,
  PCL6_E_MILLIMETER,
  PCL6_E_TENTHS_OF_A_MILLIMETER
};

enum pcl6_media_size
{
  PCL6_E_LETTER_PAPER,
  PCL6_E_LEGAL_PAPER,
  PCL6_E_A4_PAPER,
  PCL6_E_EXEC_PAPER,
  PCL6_E_LEDGER_PAPER,
  PCL6_E_A3_PAPER,
  PCL6_E_COM10_ENVELOPE,
  PCL6_E_MONARCH_ENVELOPE,
  PCL6_E_C5_ENVELOPE,
  PCL6_E_DL_ENVELOPE,
  PCL6_E_JB4_PAPER,
  PCL6_E_JB5_PAPER,
  PCL6_E_B5_ENVELOPE,
  PCL6_E_JPOSTCARD,
  PCL6_E_JDOUBLE_POSTCARD,
  PCL6_E_A5_PAPER,
  PCL6_E_A6_PAPER,
  PCL6_E_JB6_PAPER,
  PCL6_E_JIS_8K_PAPER,
  PCL6_E_JIS_16K_PAPER,
  PCL6_E_JIS_EXEC_PAPER
};

enum pcl6_media_source
{
  PCL6_E_DEFAULT_SOURCE,
  PCL6_E_AUTO_SELECT,
  PCL6_E_MANUAL_FEED,
  PCL6_E_MULTI_PURPOSE_TRAY,
  PCL6_E_UPPER_CASSETTE,
  PCL6_E_LOWER_CASSETTE,
  PCL6_E_ENVELOPE_TRAY,
  PCL6_E_THIRD_CASSETTE,
  PCL6_E_TRAY_1,
  PCL6_E_TRAY_2,
  PCL6_E_TRAY_3,
  PCL6_E_TRAY_4,
  PCL6_E_TRAY_5,
  PCL6_E_TRAY_6,
  PCL6_E_TRAY_7,
  PCL6_E_TRAY_8,
  PCL6_E_TRAY_9,
  PCL6_E_TRAY_10,
  PCL6_E_TRAY_11,
  PCL6_E_TRAY_12,
  PCL6_E_TRAY_13,
  PCL6_E_TRAY_14,
  PCL6_E_TRAY_15,
  PCL6_E_TRAY_16,
  PCL6_E_TRAY_17,
  PCL6_E_TRAY_18,
  PCL6_E_TRAY_19,
  PCL6_E_TRAY_20,
};

enum pcl6_orientation
{
  PCL6_E_PORTRAIT_ORIENTATION,
  PCL6_E_LANDSCAPE_ORIENTATION,
  PCL6_E_REVERSE_PORTRAIT,
  PCL6_E_REVERSE_LANDSCAPE
};

enum pcl6_simplex_page_mode
{
  PCL6_E_SIMPLEX_FRONT_SIDE
};
#endif // WITH_PCL6

//
// Types...
//

typedef struct pcl_s // Job data
{
  hp_driver_t driver;       // Driver to use
  size_t linesize;          // Size of output line
  unsigned width,           // Width
      height,               // Height
      xstart,               // First column on page/line
      xend,                 // Last column on page/line
      ystart,               // First line on page
      yend;                 // Last line on page
  unsigned char *planes[4], // Output buffers
      *comp_buffer;         // Compression buffer
  unsigned num_planes,      // Number of color planes
      feed;                 // Number of lines to skip
  int compression;          // Compression mode
} pcl_t;

typedef struct pcl_map_s // PCL name to code map
{
  const char *keyword; // Keyword string
  int value;           // Value
} pcl_map_t;

//
// Local globals...
//
static pappl_pr_driver_t *drivers; // Driver information

static const char *const pcl_hp_deskjet_media[] =
    { // Supported media sizes for HP Deskjet printers
        "na_legal_8.5x14in",
        "na_letter_8.5x11in",
        "na_executive_7x10in",
        "iso_a4_210x297mm",
        "iso_a5_148x210mm",
        "jis_b5_182x257mm",
        "iso_b5_176x250mm",
        "na_number-10_4.125x9.5in",
        "iso_c5_162x229mm",
        "iso_dl_110x220mm",
        "na_monarch_3.875x7.5in"};

static const char *const pcl_generic_pcl_media[] =
    { // Supported media sizes for Generic PCL printers
        "na_ledger_11x17in",
        "na_legal_8.5x14in",
        "na_letter_8.5x11in",
        "na_executive_7x10in",
        "iso_a3_297x420mm",
        "iso_a4_210x297mm",
        "iso_a5_148x210mm",
        "jis_b5_182x257mm",
        "iso_b5_176x250mm",
        "na_number-10_4.125x9.5in",
        "iso_c5_162x229mm",
        "iso_dl_110x220mm",
        "na_monarch_3.875x7.5in"};

static const char *const pcl_hp_laserjet_media[] =
    { // Supported media sizes for HP Laserjet printers
        "na_ledger_11x17in",
        "na_legal_8.5x14in",
        "na_letter_8.5x11in",
        "na_executive_7x10in",
        "iso_a3_297x420mm",
        "iso_a4_210x297mm",
        "iso_a5_148x210mm",
        "jis_b5_182x257mm",
        "iso_b5_176x250mm",
        "na_number-10_4.125x9.5in",
        "iso_c5_162x229mm",
        "iso_dl_110x220mm",
        "na_monarch_3.875x7.5in"};

//
// Local functions...
//

static const char *pcl_autoadd(const char *device_info, const char *device_uri, const char *device_id, void *data);
static bool pcl_callback(pappl_system_t *system, const char *driver_name, const char *device_uri, const char *device_id, pappl_pr_driver_data_t *driver_data, ipp_t **driver_attrs, void *data);
static void pcl_compress_data(pcl_t *pcl, pappl_device_t *device, unsigned y, const unsigned char *line, unsigned length, unsigned plane);
static bool pcl_print(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device);
static bool pcl_rendjob(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device);
static bool pcl_rendpage(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device, unsigned page);
static bool pcl_rstartjob(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device);
static bool pcl_rstartpage(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device, unsigned page);
static bool pcl_rwriteline(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device, unsigned y, const unsigned char *pixels);
static void pcl_setup(pappl_system_t *system);
static bool pcl_status(pappl_printer_t *printer);
static bool pcl_update_status(pappl_printer_t *printer, pappl_device_t *device);
static pappl_system_t *system_cb(int num_options, cups_option_t *options, void *data);
#if WITH_PCL6
static void pcl6_write_command(pappl_device_t *device, enum pcl6_cmd command);
static void pcl6_write_data(pappl_device_t *device, const unsigned char *buffer, size_t length);
// static void	pcl6_write_string(pappl_device_t *device, const char *s, enum pcl6_attr attr);
static void pcl6_write_ubyte(pappl_device_t *device, unsigned n, enum pcl6_attr attr);
static void pcl6_write_uint16(pappl_device_t *device, unsigned n, enum pcl6_attr attr);
static void pcl6_write_uint32(pappl_device_t *device, unsigned n, enum pcl6_attr attr);
static void pcl6_write_xy(pappl_device_t *device, unsigned x, unsigned y, enum pcl6_attr attr);
#endif // WITH_PCL6

//
// 'main()' - Main entry for the hp-printer-app.
//

int main(int argc,     // I - Number of command-line arguments
         char *argv[]) // I - Command-line arguments
{
  stp_init();
  return (papplMainloop(argc, argv,
                        VERSION,
                        "Copyright &copy; 2020-2022 by Michael R Sweet. Provided under the terms of the <a href=\"https://www.apache.org/licenses/LICENSE-2.0\">Apache License 2.0</a>.",
                        stp_printer_model_count(),
                        drivers, pcl_autoadd, pcl_callback,
                        /*subcmd_name*/ NULL, /*subcmd_cb*/ NULL,
                        /*system_cb*/ system_cb,
                        /*usage_cb*/ NULL,
                        /*data*/ NULL));
}

//
// 'driver_list_setup' - Populate Drivers' list
//

void driver_list_setup()
{
  const stp_printer_t *printer;
  int drivers_count = stp_printer_model_count();
  drivers = (pappl_pr_driver_t *)calloc(drivers_count, sizeof(pappl_pr_driver_t));
  for (int i = 0; i < drivers_count; i++)
  {
    if ((printer = stp_get_printer_by_index(i)) != NULL)
    {
      const char *device_id = stp_printer_get_device_id(printer);
      const char *driver_name = stp_printer_get_driver(printer);
      const char *des = stp_printer_get_long_name(printer);
      drivers[i].device_id = device_id;
      drivers[i].description = des;
      drivers[i].name = driver_name;
      drivers[i].extension = NULL;
    }
  }
}

//
// 'system_cb()' - System Callback
//

static pappl_system_t * // O - System object
system_cb(
    int num_options,        // I - Number options
    cups_option_t *options, // I - Options
    void *data)             // I - Callback data (unused)
{
  pappl_system_t *system;    // System object
  const char *val,           // Current option value
      *hostname,             // Hostname, if any
      *logfile,              // Log file, if any
      *system_name;          // System name, if any
  pappl_loglevel_t loglevel; // Log level
  int port = 0;              // Port number, if any
  pappl_soptions_t soptions = PAPPL_SOPTIONS_MULTI_QUEUE | PAPPL_SOPTIONS_WEB_INTERFACE | PAPPL_SOPTIONS_WEB_LOG | PAPPL_SOPTIONS_WEB_SECURITY;

  // Enable/disable TLS...
  if ((val = cupsGetOption("tls", num_options, options)) != NULL && !strcmp(val, "no"))
    soptions |= PAPPL_SOPTIONS_NO_TLS;

  // Parse standard log and server options...
  if ((val = cupsGetOption("log-level", num_options, options)) != NULL)
  {
    if (!strcmp(val, "fatal"))
      loglevel = PAPPL_LOGLEVEL_FATAL;
    else if (!strcmp(val, "error"))
      loglevel = PAPPL_LOGLEVEL_ERROR;
    else if (!strcmp(val, "warn"))
      loglevel = PAPPL_LOGLEVEL_WARN;
    else if (!strcmp(val, "info"))
      loglevel = PAPPL_LOGLEVEL_INFO;
    else if (!strcmp(val, "debug"))
      loglevel = PAPPL_LOGLEVEL_DEBUG;
    else
    {
      fprintf(stderr, "lprint: Bad log-level value '%s'.\n", val);
      return (NULL);
    }
  }
  else
    loglevel = PAPPL_LOGLEVEL_UNSPEC;

  logfile = cupsGetOption("log-file", num_options, options);
  hostname = cupsGetOption("server-hostname", num_options, options);
  system_name = cupsGetOption("system-name", num_options, options);

  if ((val = cupsGetOption("server-port", num_options, options)) != NULL)
  {
    if (!isdigit(*val & 255))
    {
      fprintf(stderr, "lprint: Bad server-port value '%s'.\n", val);
      return (NULL);
    }
    else
      port = atoi(val);
  }

  // Create the system object...
  if ((system = papplSystemCreate(soptions, system_name ? system_name : "LPrint", port, "_print,_universal", cupsGetOption("spool-directory", num_options, options), logfile ? logfile : "-", loglevel, cupsGetOption("auth-service", num_options, options), /* tls_only */ false)) == NULL)
    return (NULL);

  // Setup drivers list
  driver_list_setup();

  return system;
}

//
// 'pcl_autoadd()' - Auto-add PCL printers.
//

static const char *                  // O - Driver name or `NULL` for none
pcl_autoadd(const char *device_info, // I - Device name
            const char *device_uri,  // I - Device URI
            const char *device_id,   // I - IEEE-1284 device ID
            void *data)              // I - Callback data (not used)
{
  const char *ret = NULL; // Return value
  int num_did;            // Number of device ID key/value pairs
  cups_option_t *did;     // Device ID key/value pairs

  const char *make, *model;
  num_did = papplDeviceParseID(device_id, &did);

  make = cupsGetOption("MANUFACTURER", num_did, did) != NULL ? cupsGetOption("MANUFACTURER", num_did, did) : cupsGetOption("MFG", num_did, did);
  model = cupsGetOption("MFG", num_did, did) != NULL ? cupsGetOption("MODEL", num_did, did) : cupsGetOption("MDL", num_did, did);

  for (int i = 0; i < stp_printer_model_count(); i++)
  {
    cups_option_t *driver_pairs;
    int pairs = PappleDeviceParseID(drivers[i].device_id, &driver_pairs);
    const char *d_make = cupsGetOption("MANUFACTURER", pairs, driver_pairs) != NULL ? cupsGetOption("MANUFACTURER", pairs, driver_pairs) : cupsGetOption("MFG", pairs, driver_pairs);
    const char *d_model = cupsGetOption("MFG", pairs, driver_pairs) != NULL ? cupsGetOption("MODEL", pairs, driver_pairs) : cupsGetOption("MDL", pairs, driver_pairs);
    if (!strcasecmp(make, d_make) && !strcasecmp(model, d_model))
    {
      ret = drivers[i].name;
      break;
    }
    cupsFreeOptions(pairs, driver_pairs);
  }

  cupsFreeOptions(num_did, did);

  return (ret);
}

//
// 'pcl_callback()' - PCL callback.
//

static bool // O - `true` on success, `false` on failure
pcl_callback(
    pappl_system_t *system,              // I - System
    const char *driver_name,             // I - Driver name
    const char *device_uri,              // I - Device URI (not used)
    const char *device_id,               // I - IEEE-1284 device ID (not used)
    pappl_pr_driver_data_t *driver_data, // O - Driver data
    ipp_t **driver_attrs,
    // O - Driver attributes (not used)
    void *data) // I - Callback data (not used)
{
  stp_printer_t *printer;          // Printer Model
  stp_vars_t *v;                   // var object to store printer options
  stp_parameter_t desc;            // description of a parameter
  stp_resolution_t xdpi, ydpi;     // resolution info
  stp_parameter_list_t param_list; // parameter list
  stp_dimension_t width, height,   /* Page information */
      bottom, left,
      top, right;
  stp_dimension_t min_width, /* Min/max custom size */
      min_height,
      max_width,
      max_height;
  int ipp_min_height, /* Min/max custom size in mm */
      ipp_max_height,
      ipp_min_width,
      ipp_max_width;
  int printer_is_color = 0;
  stp_dimension_t top_min = 0,
                  bottom_min = 0, right_min = 0, left_min = 0;
  stp_dimension_t dflt_left, dflt_right, dflt_top, dflt_width, dflt_height, dflt_bottom; // default media size
  const char *dflt_media_size, *dflt_media_type;                                         // store default media size and type name
  const stp_vars_t *printvars;                                                           // store default printer options
  const char *long_name;                                                                 // long name of a printer
  const stp_param_string_t *opt;                                                         // variable to store currrent option
  int num_opt;                                                                           // number of options for a parameter
  const pwg_media_t *media;                                                              // raster media structure
  bool custom_size = false;                                                              // to check if printer supports custom page size
  printer = stp_get_printer_by_driver(driver_name);
  printvars = stp_printer_get_defaults(printer);
  long_name = stp_printer_get_long_name(printer);
  v = stp_vars_create_copy(printvars);

  if (printer == NULL || long_name == NULL)
  {
    papplLog(system, PAPPL_LOGLEVEL_ERROR, "Driver name '%s' not supported.", driver_name);
    return false;
  }
  /* Set callbacks */
  driver_data->printfile_cb = NULL;
  driver_data->rendjob_cb = pcl_rendjob;
  driver_data->rendpage_cb = pcl_rendpage;
  driver_data->rstartjob_cb = pcl_rstartjob;
  driver_data->rstartpage_cb = pcl_rstartpage;
  driver_data->rwriteline_cb = pcl_rwriteline;
  driver_data->status_cb = pcl_status;
  driver_data->has_supplies = false;

  // Media Types

  stp_describe_parameter(v, "MediaType", &desc);

  if (desc.p_type == STP_PARAMETER_TYPE_STRING_LIST && desc.is_active &&
      stp_string_list_count(desc.bounds.str) > 0)
  {
    num_opt = stp_string_list_count(desc.bounds.str);

    driver_data->num_type = num_opt;

    for (int i = 0; i < num_opt; i++)
    {
      opt = stp_string_list_param(desc.bounds.str, i);
      if (strcmp(opt->name, desc.deflt.str))
      {
        dflt_media_type = pwgMediaForPPD(opt->name)->pwg;
      }
      driver_data->type[i] = pwgMediaForPPD(opt->name)->pwg;
    }
  }

  stp_parameter_description_destroy(&desc);

  // Input Slots/Media Sources

  stp_describe_parameter(v, "InputSlot", &desc);

  if (desc.p_type == STP_PARAMETER_TYPE_STRING_LIST && desc.is_active &&
      stp_string_list_count(desc.bounds.str) > 0)
  {
    num_opt = stp_string_list_count(desc.bounds.str);
    driver_data->num_source = num_opt;
    for (int i = 0; i < num_opt; i++)
    {
      opt = stp_string_list_param(desc.bounds.str, i);
      driver_data->source[i] = pwgMediaForPPD(opt->name)->pwg;
    }
  }
  stp_paremeter_description_destroy(&desc);

  // media sizes
  stp_describe_parameter(v, "PageSize", &desc);
  if (desc.p_type == STP_PARAMETER_TYPE_STRING_LIST && desc.is_active &&
      stp_string_list_count(desc.bounds.str) > 0)
  {
    int cnt = 0;
    num_opt = stp_string_list_count(desc.bounds.str);
    for (int i = 0; i < num_opt; i++)
    {
      const stp_papersize_t *papersize;
      opt = stp_string_list_param(desc.bounds.str, i);
      if (!strcmp(opt->name, "Custom"))
      {
        custom_size = true;
        continue;
      }
      papersize = stp_describe_papersize(v, opt->name);

      if (!papersize)
      {

        continue;
      }
      if (papersize->width <= 0 || papersize->height <= 0)
        continue;

      width = papersize->width;
      height = papersize->height;

      stp_set_string_parameter(v, "PageSize", opt->name);

      stp_get_media_size(v, &width, &height);
      stp_get_maximum_imageable_area(v, &left, &right, &bottom, &top);

      if (left < 0)
        left = 0;
      if (right > width)
        right = width;
      if (bottom > height)
        bottom = height;
      if (top < 0)
        top = 0;
      if (!strcmp(opt->name, desc.deflt.str))
      {
        dflt_media_size = pwgMediaForPPD(opt->name)->pwg;
        dflt_bottom = bottom;
        dflt_top = top;
        dflt_right = right;
        dflt_width = width;
        dflt_height = height;
      }
      if (left_min > left)
      {
        left_min = left;
      }
      if (right_min > right)
      {
        right_min = right;
      }
      if (top_min > (height - top))
      {
        top_min = height - top;
      }
      if (bottom_min > (height - bottom))
      {
        bottom_min = height - bottom;
      }
      cnt++;
      stp_clear_string_parameter(v, "PageSize");
    }
    if (custom_size)
    {
      cnt += 2;
    }
    driver_data->num_media = cnt;
    cnt = 0;
    for (int i = 0; i < num_opt; i++)
    {
      opt = stp_string_list_param(desc.bounds.str, i);
      if (!strcmp(opt->name, "Custom"))
      {
        continue;
      }
      const pwg_media_t *m = pwgMediaForPPD(opt->name);
      driver_data->media[cnt++] = m->pwg;
    }
    if (custom_size)
    {
      stp_set_string_parameter(v, "PageSize", "Custom");
      stp_get_media_size(v, &width, &height);
      stp_get_maximum_imageable_area(v, &left, &right, &bottom, &top);

      stp_get_size_limit(v, &max_width, &max_height, &min_width, &min_height);
      if (left < 0)
        left = 0;
      if (top < 0)
        top = 0;
      if (bottom > height)
        bottom = height;
      if (right > width)
        width = right;
      if (left_min > left)
      {
        left_min = left;
      }
      if (right_min > right)
      {
        right_min = right;
      }
      if (top_min > (height - top))
      {
        top_min = height - top;
      }
      if (bottom_min > (height - bottom))
      {
        bottom_min = height - bottom;
      }

      ipp_min_height = (int)(min_height / 72.0 * 2540.0) / 1000;
      ipp_max_height = (int)(max_height / 72.0 * 2540.0) / 1000;
      ipp_min_width = (int)(min_width / 72.0 * 2540.0) / 1000;
      ipp_max_width = (int)(max_width / 72.0 * 2540.0) / 1000;

      const char *max_dimension[256], *min_dimension[256];

      snprintf(max_dimension, sizeof(max_dimension), "custom_max_%dx%d%s", ipp_max_width, max_height, "mm");
      snprintf(min_dimension, sizeof(min_dimension), "custom_min_%dx%d%s", ipp_max_width, max_height, "mm");
      driver_data->media[cnt++] = max_dimension;
      driver_data->media[cnt++] = min_dimension;
      stp_clear_string_parameter(v, "PageSize");
    }
  }
  stp_parameter_description_destroy(&desc);

  // resolution

  stp_describe_parameter(v, "Resolution", &desc);

  if (desc.p_type == STP_PARAMETER_TYPE_STRING_LIST && desc.is_active)
  {
    num_opt = stp_string_list_count(desc.bounds.str);
    driver_data->num_resolution = 3;
    int max_xdpi = -1, max_ydpi = -1;
    for (int i = 0; i < num_opt; i++)
    {
      opt = stp_string_list_param(desc.bounds.str, i);
      stp_set_string_parameter(v, "Resolution", opt->name);
      stp_describe_resolution(v, &xdpi, &ydpi);
      // find the highest resolution printer supports
      if (xdpi != -1 && ydpi != -1)
      {
        if (xdpi > max_xdpi)
        {
          max_xdpi = xdpi;
        }
        if (ydpi > max_ydpi)
        {
          max_ydpi = ydpi;
        }
      }
    }

    if (max_xdpi != -1 || max_ydpi != -1)
    {
      // raising the lower resolution to make symmetric resolution
      if (max_xdpi > max_ydpi)
      {
        max_ydpi = max_xdpi;
      }
      else
      {
        max_xdpi = max_ydpi;
      }

      // limit the resolution to 1440 for 360 dpi based and 1200 for 300 dpi

      while (max_xdpi > 1440)
      {
        max_xdpi /= 2;
        max_ydpi /= 2;
      }

      driver_data->x_resolution[0] = (int)max_xdpi / 4;
      driver_data->y_resolution[0] = (int)max_xdpi / 4;
      driver_data->x_resolution[1] = (int)max_xdpi / 2;
      driver_data->y_resolution[1] = (int)max_xdpi / 2;
      driver_data->x_resolution[2] = max_xdpi;
      driver_data->y_resolution[2] = max_xdpi;
      driver_data->x_default = driver_data->y_default = (int)max_xdpi / 2;
    }
    stp_clear_string_parameter(v, "Resolution");
  }
  stp_parameter_description_destroy(&desc);

  // setting the margins
  if (left_min < right_min)
  {
    driver_data->left_right = (int)(left_min / 72.0 * 2540.0) / 10;
  }
  else
  {
    driver_data->left_right = (int)(right_min / 72.0 * 2540.0) / 10;
  }
  if (top_min < bottom_min)
  {
    driver_data->left_right = (int)(top_min / 72.0 * 2540.0) / 10;
  }
  else
  {
    driver_data->left_right = (int)(bottom_min / 72.0 * 2540.0) / 10;
  }

  (void)data;
  (void)device_uri;
  (void)driver_attrs;

  /* Native format */
  driver_data->format = "application/vnd.printer-specific";

  /* Default orientation and quality */
  driver_data->orient_default = IPP_ORIENT_NONE;
  driver_data->quality_default = IPP_QUALITY_NORMAL;

  // Set the make and model

  papplCopyString(driver_data->make_and_model, long_name, sizeof(driver_data->make_and_model));

  // set the printer color mode
  stp_describe_parameter(v, "PrintingMode", &desc);

  if (stp_string_list_is_present(desc.bounds.str, "Color"))
  {
    printer_is_color = 1;
  }
  else
  {
    printer_is_color = 0;
  }
  if (strcmp(desc.deflt.str, "Color") == 0)
  {
    driver_data->color_default = PAPPL_COLOR_MODE_COLOR;
  }
  else
  {
    driver_data->color_default = PAPPL_COLOR_MODE_MONOCHROME;
  }

  if (printer_is_color)
  {
    driver_data->raster_types = PAPPL_PWG_RASTER_TYPE_BLACK_1 | PAPPL_PWG_RASTER_TYPE_BLACK_8 | PAPPL_PWG_RASTER_TYPE_SGRAY_8 | PAPPL_PWG_RASTER_TYPE_SRGB_8;

    driver_data->color_supported = PAPPL_COLOR_MODE_MONOCHROME | PAPPL_COLOR_MODE_AUTO | PAPPL_COLOR_MODE_COLOR;
  }
  else
  {
    driver_data->raster_types = PAPPL_PWG_RASTER_TYPE_BLACK_1 | PAPPL_PWG_RASTER_TYPE_BLACK_8 | PAPPL_PWG_RASTER_TYPE_SGRAY_8;

    driver_data->color_supported = PAPPL_COLOR_MODE_MONOCHROME;
  }
  stp_parameter_description_destroy(&desc);

  /* Pages-per-minute for monochrome and color */
  driver_data->ppm = 1;
  driver_data->ppm_color = 1;

  // Duplex Mode
  driver_data->sides_supported = PAPPL_SIDES_ONE_SIDED;
  stp_describe_parameter(v, "Duplex", &desc);
  if (desc.is_active && desc.p_type == STP_PARAMETER_TYPE_STRING_LIST)
  {
    num_opt = stp_string_list_count(desc.bounds.str);
    if (num_opt > 0)
    {
      if (!strcmp(desc.deflt.str, "None"))
      {
        driver_data->sides_default = PAPPL_SIDES_ONE_SIDED;
      }
      else if (!strcmp(desc.deflt.str, "DuplexTumble"))
      {
        driver_data->sides_default = PAPPL_SIDES_TWO_SIDED_SHORT_EDGE;
      }
      else if (!strcmp(desc.deflt.str, "DuplexNoTumble"))
      {
        driver_data->sides_default = PAPPL_SIDES_TWO_SIDED_LONG_EDGE;
      }
      else
      {
        driver_data->sides_default = PAPPL_SIDES_ONE_SIDED;
      }
      if (num_opt == 1)
      {
        opt = stp_string_list_param(desc.bounds.str, 0);
        if (strcmp(opt->name, "None") == 0)
        {
          driver_data->sides_supported = PAPPL_SIDES_ONE_SIDED;
          driver_data->duplex = PAPPL_DUPLEX_NONE;
        }
      }
      else
      {
        for (int i = 0; i < num_opt; i++)
        {
          opt = stp_string_list_param(desc.bounds.str, i);
          if (strcmp(opt->name, "DuplexTumble") == 0)
          {
            driver_data->sides_supported = driver_data->sides_supported | PAPPL_SIDES_TWO_SIDED_SHORT_EDGE;
            driver_data->duplex = driver_data->duplex | PAPPL_DUPLEX_FLIPPED;
          }
          else if (strcmp(opt->name, "DuplexNoTumble") == 0)
          {
            driver_data->sides_supported = driver_data->sides_supported | PAPPL_SIDES_TWO_SIDED_LONG_EDGE;
            driver_data->duplex = driver_data->duplex | PAPPL_DUPLEX_NORMAL;
          }
        }
      }
    }
    else
    {
      driver_data->sides_supported = PAPPL_SIDES_ONE_SIDED;
      driver_data->duplex = PAPPL_DUPLEX_NONE;
    }
  }
  stp_parameter_description_destroy(&desc);
  driver_data->icons[0].data = hp_deskjet_sm_png;
  driver_data->icons[0].datalen = sizeof(hp_deskjet_sm_png);

  // Fill out ready and default media (default == ready media from the first source)

  for (int i = 0; i < driver_data->num_source; i++)
  {
    pwg_media_t *pwg; /* Media size information */

    /* Use Default Media Size */
    papplCopyString(driver_data->media_ready[i].size_name, dflt_media_size, sizeof(driver_data->media_ready[i].size_name));
    // if (!strcmp(driver_data->source[i], "envelope"))
    //   papplCopyString(driver_data->media_ready[i].size_name, "na_number-10_4.125x9.5in", sizeof(driver_data->media_ready[i].size_name));
    // else
    //   papplCopyString(driver_data->media_ready[i].size_name, "na_letter_8.5x11in", sizeof(driver_data->media_ready[i].size_name));

    /* Set margin and size information */

    driver_data->media_ready[i].bottom_margin = (int)(dflt_bottom / 72.0 * 2540.0) / 10;
    driver_data->media_ready[i].left_margin = (int)(dflt_left / 72.0 * 2540.0) / 10;
    driver_data->media_ready[i].right_margin = (int)(dflt_right / 72.0 * 2540.0) / 10;
    driver_data->media_ready[i].size_width = (int)(dflt_width / 72.0 * 2540.0) / 10;
    driver_data->media_ready[i].size_length = (int)(dflt_height / 72.0 * 2540.0) / 10;
    driver_data->media_ready[i].top_margin = (int)(dflt_top / 72.0 * 2540.0) / 10;
    papplCopyString(driver_data->media_ready[i].source, driver_data->source[i], sizeof(driver_data->media_ready[i].source));
    papplCopyString(driver_data->media_ready[i].type, dflt_media_type, sizeof(driver_data->media_ready[i].type));
  }

  driver_data->media_default = driver_data->media_ready[0];

  return (true);
}

//
// 'pcl_compress_data()' - Compress a line of graphics.
//

static void
pcl_compress_data(
    pcl_t *pcl,                // I - Job data
    pappl_device_t *device,    // I - Device
    unsigned y,                // I - Line number
    const unsigned char *line, // I - Data to compress
    unsigned length,           // I - Number of bytes
    unsigned plane)            // I - Color plane
{
  const unsigned char *line_ptr, // Current byte pointer
      *line_end,                 // End-of-line byte pointer
      *start;                    // Start of compression sequence
  unsigned char *comp_ptr;       // Pointer into compression buffer
  unsigned count;                // Count of bytes for output
  int comp;                      // Current compression type

  // Try doing TIFF PackBits compression...
  line_ptr = line;
  line_end = line + length;
  comp_ptr = pcl->comp_buffer;

  while (line_ptr < line_end)
  {
    if ((line_ptr + 1) >= line_end)
    {
      // Single byte on the end...
      *comp_ptr++ = 0x00;
      *comp_ptr++ = *line_ptr++;
    }
    else if (line_ptr[0] == line_ptr[1])
    {
      // Repeated sequence...
      line_ptr++;
      count = 2;

      while (line_ptr < (line_end - 1) && line_ptr[0] == line_ptr[1] && count < 128)
      {
        line_ptr++;
        count++;
      }

      *comp_ptr++ = (unsigned char)(257 - count);
      *comp_ptr++ = *line_ptr++;
    }
    else
    {
      // Non-repeated sequence...
      start = line_ptr;
      line_ptr++;
      count = 1;

      while (line_ptr < (line_end - 1) && line_ptr[0] != line_ptr[1] && count < 128)
      {
        line_ptr++;
        count++;
      }

      *comp_ptr++ = (unsigned char)(count - 1);

      memcpy(comp_ptr, start, count);
      comp_ptr += count;
    }
  }

  if ((unsigned)(comp_ptr - pcl->comp_buffer) > length)
  {
    // Don't try compressing...
    comp = 0;
  }
  else
  {
    // Use PackBits compression...
    comp = 2;
    line_ptr = pcl->comp_buffer;
    line_end = comp_ptr;
  }

  switch (pcl->driver)
  {
  case HP_DRIVER_DESKJET:
  case HP_DRIVER_GENERIC:
  case HP_DRIVER_LASERJET:
    // Set compression mode as needed...
    if (pcl->compression != comp)
    {
      // Set compression
      pcl->compression = comp;
      papplDevicePrintf(device, "\033*b%uM", pcl->compression);
    }

    // Set the length of the data and write a raster plane...
    papplDevicePrintf(device, "\033*b%d%c", (int)(line_end - line_ptr), plane < (pcl->num_planes - 1) ? 'V' : 'W');
    papplDeviceWrite(device, line_ptr, (size_t)(line_end - line_ptr));
    break;

#if WITH_PCL6
  case HP_DRIVER_GENERIC6:
  case HP_DRIVER_GENERIC6C:
    count = (unsigned)(line_end - line_ptr);
    if (!comp)
      count = (count + 3) & ~3; // Pad to 32-bit boundary

    pcl6_write_uint16(device, y - pcl->ystart, PCL6_ATTR_START_LINE);
    pcl6_write_uint16(device, 1, PCL6_ATTR_BLOCK_HEIGHT);
    pcl6_write_ubyte(device, comp ? PCL6_E_RLE_COMPRESSION : PCL6_E_NO_COMPRESSION, PCL6_ATTR_COMPRESS_MODE);
    //	pcl6_write_ubyte(device, 1, PCL6_ATTR_PAD_BYTES_MULTIPLE);
    //        pcl6_write_uint32(device, count, PCL6_ATTR_BLOCK_BYTE_LENGTH);
    pcl6_write_command(device, PCL6_CMD_READ_IMAGE);
    pcl6_write_data(device, line_ptr, count);
    break;
#endif // WITH_PCL6
  }
}

//
// 'pcl_print()' - Print file.
//

static bool // O - `true` on success, `false` on failure
pcl_print(
    pappl_job_t *job,            // I - Job
    pappl_pr_options_t *options, // I - Options
    pappl_device_t *device)      // I - Device
{
  int fd;             // Job file
  ssize_t bytes;      // Bytes read/written
  char buffer[65536]; // Read/write buffer

  papplLogJob(job, PAPPL_LOGLEVEL_DEBUG, "Printing raw file...");

  papplJobSetImpressions(job, 1);

  fd = open(papplJobGetFilename(job), O_RDONLY);

  while ((bytes = read(fd, buffer, sizeof(buffer))) > 0)
  {
    if (papplDeviceWrite(device, buffer, (size_t)bytes) < 0)
    {
      papplLogJob(job, PAPPL_LOGLEVEL_ERROR, "Unable to send %d bytes to printer.", (int)bytes);
      close(fd);
      return (false);
    }
  }

  close(fd);

  papplJobSetImpressionsCompleted(job, 1);

  return (true);
}

//
// 'pcl_rendjob()' - End a job.
//

static bool // O - `true` on success, `false` on failure
pcl_rendjob(
    pappl_job_t *job,            // I - Job
    pappl_pr_options_t *options, // I - Options
    pappl_device_t *device)      // I - Device
{
  pcl_t *pcl = (pcl_t *)papplJobGetData(job);
  // Job data

  papplLogJob(job, PAPPL_LOGLEVEL_DEBUG, "Ending job...");

  (void)options;

  switch (pcl->driver)
  {
  case HP_DRIVER_DESKJET:
  case HP_DRIVER_GENERIC:
  case HP_DRIVER_LASERJET:
    papplDevicePuts(device, "\033E");
    break;

#if WITH_PCL6
  case HP_DRIVER_GENERIC6:
  case HP_DRIVER_GENERIC6C:
    pcl6_write_command(device, PCL6_CMD_END_SESSION);
    papplDevicePuts(device, "\033%-12345X");
    break;
#endif // WITH_PCL6
  }

  free(pcl);
  papplJobSetData(job, NULL);

  pcl_update_status(papplJobGetPrinter(job), device);

  return (true);
}

//
// 'pcl_rendpage()' - End a page.
//

static bool // O - `true` on success, `false` on failure
pcl_rendpage(
    pappl_job_t *job,            // I - Job
    pappl_pr_options_t *options, // I - Job options
    pappl_device_t *device,      // I - Device
    unsigned page)               // I - Page number
{
  pcl_t *pcl = (pcl_t *)papplJobGetData(job);
  // Job data

  papplLogJob(job, PAPPL_LOGLEVEL_DEBUG, "Ending page %u...", page);

  switch (pcl->driver)
  {
  case HP_DRIVER_DESKJET:
  case HP_DRIVER_GENERIC:
  case HP_DRIVER_LASERJET:
    // Eject the current page...
    if (pcl->num_planes > 1)
    {
      papplDevicePuts(device, "\033*rC"); // End color GFX

      if (!(options->header.Duplex && (page & 1)))
        papplDevicePuts(device, "\033&l0H");
      // Eject current page
    }
    else
    {
      papplDevicePuts(device, "\033*r0B"); // End GFX

      if (!(options->header.Duplex && (page & 1)))
        papplDevicePuts(device, "\014"); // Eject current page
    }
    break;

#if WITH_PCL6
  case HP_DRIVER_GENERIC6:
  case HP_DRIVER_GENERIC6C:
    pcl6_write_command(device, PCL6_CMD_END_IMAGE);
    pcl6_write_command(device, PCL6_CMD_CLOSE_DATA_SOURCE);
    pcl6_write_command(device, PCL6_CMD_END_PAGE);
    break;
#endif // WITH_PCL6
  }

  papplDeviceFlush(device);

  // Free memory...
  free(pcl->planes[0]);
  free(pcl->comp_buffer);

  return (true);
}

//
// 'pcl_rstartjob()' - Start a job.
//

static bool // O - `true` on success, `false` on failure
pcl_rstartjob(
    pappl_job_t *job,            // I - Job
    pappl_pr_options_t *options, // I - Job options
    pappl_device_t *device)      // I - Device
{
  int i; // Looping var
  pcl_t *pcl = (pcl_t *)calloc(1, sizeof(pcl_t));
  // Job data
  const char *name = papplPrinterGetDriverName(papplJobGetPrinter(job));
  // Driver name

  papplLogJob(job, PAPPL_LOGLEVEL_DEBUG, "Starting job...");

  pcl_update_status(papplJobGetPrinter(job), device);

  (void)options;

  // Save driver type...
  pcl->driver = HP_DRIVER_GENERIC;

  for (i = 0; i < (int)(sizeof(drivers) / sizeof(drivers[0])); i++)
  {
    if (!strcmp(name, drivers[i].name))
    {
      pcl->driver = (hp_driver_t)i;
      break;
    }
  }

  papplJobSetData(job, pcl);

  switch (pcl->driver)
  {
  case HP_DRIVER_DESKJET:
  case HP_DRIVER_LASERJET:
  case HP_DRIVER_GENERIC:
    // Send a PCL reset sequence
    papplDevicePuts(device, "\033E");
    break;

#if WITH_PCL6
  case HP_DRIVER_GENERIC6:
  case HP_DRIVER_GENERIC6C:
    // Send a PCL XL start sequence
    papplDevicePuts(device, "\033%-12345X@PJL ENTER LANGUAGE = PCLXL\r\n");

    // Send a PCL XL binary stream header
    papplDevicePuts(device, ") HP-PCL XL;2;0\r\n");

    // Start PCL 6 session...
    pcl6_write_ubyte(device, PCL6_E_INCH, PCL6_ATTR_MEASURE);
    pcl6_write_xy(device, options->printer_resolution[0], options->printer_resolution[1], PCL6_ATTR_UNITS_PER_MEASURE);
    pcl6_write_ubyte(device, PCL6_E_ERROR_PAGE, PCL6_ATTR_ERROR_REPORT);
    pcl6_write_command(device, PCL6_CMD_BEGIN_SESSION);
    break;
#endif // WITH_PCL6
  }

  return (true);
}

//
// 'pcl_rstartpage()' - Start a page.
//

static bool // O - `true` on success, `false` on failure
pcl_rstartpage(
    pappl_job_t *job,            // I - Job
    pappl_pr_options_t *options, // I - Job options
    pappl_device_t *device,      // I - Device
    unsigned page)               // I - Page number
{
  size_t i;       // Looping var
  unsigned plane, // Looping var
      length;     // Bytes to write
  cups_page_header_t *header = &(options->header);
  // Page header
  pcl_t *pcl = (pcl_t *)papplJobGetData(job);
  // Job data
  static const pcl_map_t pcl_sizes[] = // PCL media size values
      {
          {"iso_a3_297x420mm", 27},
          {"iso_a4_210x297mm", 26},
          {"iso_a5_148x210mm", 25},
          {"iso_b5_176x250mm", 100},
          {"iso_c5_162x229mm", 91},
          {"iso_dl_110x220mm", 90},
          {"jis_b5_182x257mm", 45},
          {"na_executive_7x10in", 1},
          {"na_ledger_11x17in", 6},
          {"na_legal_8.5x14in", 3},
          {"na_letter_8.5x11in", 2},
          {"na_monarch_3.875x7.5in", 80},
          {"na_number-10_4.125x9.5in", 81}};
  static const pcl_map_t pcl_sources[] = // PCL media source values
      {
          {"auto", 7},
          {"by-pass-tray", 4},
          {"disc", 14},
          {"envelope", 6},
          {"large-capacity", 5},
          {"main", 1},
          {"manual", 2},
          {"right", 8},
          {"tray-1", 20},
          {"tray-2", 21},
          {"tray-3", 22},
          {"tray-4", 23},
          {"tray-5", 24},
          {"tray-6", 25},
          {"tray-7", 26},
          {"tray-8", 27},
          {"tray-9", 28},
          {"tray-10", 29},
          {"tray-11", 30},
          {"tray-12", 31},
          {"tray-13", 32},
          {"tray-14", 33},
          {"tray-15", 34},
          {"tray-16", 35},
          {"tray-17", 36},
          {"tray-18", 37},
          {"tray-19", 38},
          {"tray-20", 39}};
  static const pcl_map_t pcl_types[] = // PCL media type values
      {
          {"disc", 7},
          {"photographic", 3},
          {"stationery-inkjet", 2},
          {"stationery", 0},
          {"transparency", 4}};
#if WITH_PCL6
  static const pcl_map_t pcl6_sizes[] = // PCL media size values
      {
          {"iso_a3_297x420mm", PCL6_E_A3_PAPER},
          {"iso_a4_210x297mm", PCL6_E_A4_PAPER},
          {"iso_a5_148x210mm", PCL6_E_A5_PAPER},
          {"iso_b5_176x250mm", PCL6_E_B5_ENVELOPE},
          {"iso_c5_162x229mm", PCL6_E_C5_ENVELOPE},
          {"iso_dl_110x220mm", PCL6_E_DL_ENVELOPE},
          {"jis_b5_182x257mm", PCL6_E_JB5_PAPER},
          {"na_executive_7x10in", PCL6_E_EXEC_PAPER},
          {"na_ledger_11x17in", PCL6_E_LEDGER_PAPER},
          {"na_legal_8.5x14in", PCL6_E_LEGAL_PAPER},
          {"na_letter_8.5x11in", PCL6_E_LETTER_PAPER},
          {"na_monarch_3.875x7.5in", PCL6_E_MONARCH_ENVELOPE},
          {"na_number-10_4.125x9.5in", PCL6_E_COM10_ENVELOPE}};
  static const pcl_map_t pcl6_sources[] = // PCL media source values
      {
          {"auto", PCL6_E_AUTO_SELECT},
          {"by-pass-tray", PCL6_E_MULTI_PURPOSE_TRAY},
          {"envelope", PCL6_E_ENVELOPE_TRAY},
          {"large-capacity", PCL6_E_LOWER_CASSETTE},
          {"main", PCL6_E_UPPER_CASSETTE},
          {"manual", PCL6_E_MANUAL_FEED},
          {"right", PCL6_E_THIRD_CASSETTE},
          {"tray-1", PCL6_E_TRAY_1},
          {"tray-2", PCL6_E_TRAY_2},
          {"tray-3", PCL6_E_TRAY_3},
          {"tray-4", PCL6_E_TRAY_4},
          {"tray-5", PCL6_E_TRAY_5},
          {"tray-6", PCL6_E_TRAY_6},
          {"tray-7", PCL6_E_TRAY_7},
          {"tray-8", PCL6_E_TRAY_8},
          {"tray-9", PCL6_E_TRAY_9},
          {"tray-10", PCL6_E_TRAY_10},
          {"tray-11", PCL6_E_TRAY_11},
          {"tray-12", PCL6_E_TRAY_12},
          {"tray-13", PCL6_E_TRAY_13},
          {"tray-14", PCL6_E_TRAY_14},
          {"tray-15", PCL6_E_TRAY_15},
          {"tray-16", PCL6_E_TRAY_16},
          {"tray-17", PCL6_E_TRAY_17},
          {"tray-18", PCL6_E_TRAY_18},
          {"tray-19", PCL6_E_TRAY_19},
          {"tray-20", PCL6_E_TRAY_20}};
#endif // WITH_PCL6

  papplLogJob(job, PAPPL_LOGLEVEL_DEBUG, "Starting page %u...", page);

  // Setup size based on margins...
  pcl->width = options->printer_resolution[0] * (options->media.size_width - options->media.left_margin - options->media.right_margin) / 2540;
  pcl->height = options->printer_resolution[1] * (options->media.size_length - options->media.top_margin - options->media.bottom_margin) / 2540;
  pcl->xstart = options->printer_resolution[0] * options->media.left_margin / 2540;
  pcl->xend = pcl->xstart + pcl->width;
  pcl->ystart = options->printer_resolution[1] * options->media.top_margin / 2540;
  pcl->yend = pcl->ystart + pcl->height;

  switch (pcl->driver)
  {
  case HP_DRIVER_DESKJET:
  case HP_DRIVER_GENERIC:
  case HP_DRIVER_LASERJET:
    // Setup printer/job attributes...
    if (options->sides == PAPPL_SIDES_ONE_SIDED || (page & 1))
    {
      // Set media position
      for (i = 0; i < (sizeof(pcl_sources) / sizeof(pcl_sources[0])); i++)
      {
        if (!strcmp(options->media.source, pcl_sources[i].keyword))
        {
          papplDevicePrintf(device, "\033&l%dH", pcl_sources[i].value);
          break;
        }
      }

      // Set 6 LPI, 10 CPI
      papplDevicePuts(device, "\033&l6D\033&k12H");

      // Set portrait orientation
      papplDevicePuts(device, "\033&l0O");

      // Set page size
      for (i = 0; i < (sizeof(pcl_sizes) / sizeof(pcl_sizes[0])); i++)
      {
        if (!strcmp(options->media.size_name, pcl_sizes[i].keyword))
        {
          papplDevicePrintf(device, "\033&l%dA", pcl_sizes[i].value);
          break;
        }
      }

      if (i >= (sizeof(pcl_sizes) / sizeof(pcl_sizes[0])))
      {
        // Custom size, set page length...
        papplDevicePrintf(device, "\033&l%dP", 6 * options->media.size_length / 2540);
      }

      // Set media type
      for (i = 0; i < (sizeof(pcl_types) / sizeof(pcl_types[0])); i++)
      {
        if (!strcmp(options->media.type, pcl_types[i].keyword))
        {
          papplDevicePrintf(device, "\033&l%dM", pcl_types[i].value);
          break;
        }
      }

      // Set top margin to 0
      papplDevicePuts(device, "\033&l0E");

      // Turn off perforation skip
      papplDevicePuts(device, "\033&l0L");

      // Set duplex mode...
      switch (options->sides)
      {
      case PAPPL_SIDES_ONE_SIDED:
        papplDevicePuts(device, "\033&l0S");
        break;
      case PAPPL_SIDES_TWO_SIDED_LONG_EDGE:
        papplDevicePuts(device, "\033&l2S");
        break;
      case PAPPL_SIDES_TWO_SIDED_SHORT_EDGE:
        papplDevicePuts(device, "\033&l1S");
        break;
      }
    }
    else
    {
      // Set back side
      papplDevicePuts(device, "\033&a2G");
    }

    // DeskJet-specific commands...
    if (pcl->driver == HP_DRIVER_DESKJET)
    {
      // Set print quality...
      if (options->print_quality == IPP_QUALITY_HIGH || header->HWResolution[0] > 300)
        papplDevicePuts(device, "\033*o2M");
      else
        papplDevicePuts(device, "\033*o0M");

      // Handle duplexing...
      if (options->sides != PAPPL_SIDES_ONE_SIDED)
      {
        // Load media
        papplDevicePuts(device, "\033&l-2H");

        if (page & 1)
        {
          // Set duplex mode
          papplDevicePuts(device, "\033&l2S");
        }
      }
    }

    // Set resolution
    papplDevicePrintf(device, "\033*t%uR", header->HWResolution[0]);

    // Set graphics mode
    if (header->cupsColorSpace == CUPS_CSPACE_SRGB)
    {
      // KCMY
      pcl->num_planes = 4;
      papplDevicePuts(device, "\033*r-4U");
    }
    else
    {
      // K
      pcl->num_planes = 1;
    }

    // Set size
    papplDevicePrintf(device, "\033*r%uS\033*r%uT", pcl->width, pcl->height);

    // Set position
    papplDevicePrintf(device, "\033&a0H\033&a%.0fV", 720.0 * options->media.top_margin / 2540.0);

    // Start graphics
    papplDevicePuts(device, "\033*r1A");

    // Allocate dithering plane buffers
    pcl->linesize = (pcl->width + 7) / 8;

    if ((pcl->planes[0] = malloc(pcl->linesize * pcl->num_planes)) == NULL)
    {
      papplLogJob(job, PAPPL_LOGLEVEL_ERROR, "Memory allocation failure.");
      return (false);
    }

    for (plane = 1; plane < pcl->num_planes; plane++)
      pcl->planes[plane] = pcl->planes[0] + plane * pcl->linesize;
    break;

#if WITH_PCL6
  case HP_DRIVER_GENERIC6:
  case HP_DRIVER_GENERIC6C:
    if (pcl->width & 3)
    {
      papplLogJob(job, PAPPL_LOGLEVEL_DEBUG, "Adjusting width to multiple of 4 bytes.");
      pcl->width &= ~3;
      pcl->xend = pcl->xstart + pcl->width;
    }

    pcl6_write_ubyte(device, PCL6_E_PORTRAIT_ORIENTATION, PCL6_ATTR_ORIENTATION);

    // Set media position
    for (i = 0; i < (sizeof(pcl6_sources) / sizeof(pcl6_sources[0])); i++)
    {
      if (!strcmp(options->media.source, pcl6_sources[i].keyword))
      {
        pcl6_write_ubyte(device, pcl_sources[i].value, PCL6_ATTR_MEDIA_SOURCE);
        break;
      }
    }

    // Set page size
    for (i = 0; i < (sizeof(pcl6_sizes) / sizeof(pcl6_sizes[0])); i++)
    {
      if (!strcmp(options->media.size_name, pcl6_sizes[i].keyword))
      {
        pcl6_write_ubyte(device, pcl_sizes[i].value, PCL6_ATTR_MEDIA_SIZE);
        break;
      }
    }

    if (options->sides != PAPPL_SIDES_ONE_SIDED)
    {
      if (options->sides == PAPPL_SIDES_TWO_SIDED_LONG_EDGE)
        pcl6_write_ubyte(device, PCL6_E_DUPLEX_HORIZONTAL_BINDING, PCL6_ATTR_DUPLEX_PAGE_MODE);
      else
        pcl6_write_ubyte(device, PCL6_E_DUPLEX_VERTICAL_BINDING, PCL6_ATTR_DUPLEX_PAGE_MODE);

      pcl6_write_ubyte(device, (page & 1) ? PCL6_E_BACK_MEDIA_SIDE : PCL6_E_FRONT_MEDIA_SIDE, PCL6_ATTR_DUPLEX_PAGE_SIDE);
    }
    else
      pcl6_write_ubyte(device, PCL6_E_SIMPLEX_FRONT_SIDE, PCL6_ATTR_SIMPLEX_PAGE_MODE);

    pcl6_write_command(device, PCL6_CMD_BEGIN_PAGE);

    switch (header->cupsColorSpace)
    {
    case CUPS_CSPACE_K:
    case CUPS_CSPACE_W:
    case CUPS_CSPACE_SW:
      pcl6_write_ubyte(device, PCL6_E_GRAY, PCL6_ATTR_COLOR_SPACE);
      break;
    case CUPS_CSPACE_RGB:
    case CUPS_CSPACE_SRGB:
    default:
      pcl6_write_ubyte(device, PCL6_E_RGB, PCL6_ATTR_COLOR_SPACE);
      break;
    }

    pcl6_write_command(device, PCL6_CMD_SET_COLOR_SPACE);

    pcl6_write_xy(device, options->printer_resolution[0] * options->media.left_margin / 2540, options->printer_resolution[1] * options->media.top_margin / 2540, PCL6_ATTR_POINT);
    pcl6_write_command(device, PCL6_CMD_SET_CURSOR);

    pcl6_write_ubyte(device, PCL6_E_DEFAULT, PCL6_ATTR_SOURCE_TYPE);
    pcl6_write_ubyte(device, PCL6_E_BINARY_HIGH_BYTE_FIRST, PCL6_ATTR_DATA_ORG);
    pcl6_write_command(device, PCL6_CMD_OPEN_DATA_SOURCE);

    pcl6_write_ubyte(device, header->cupsBitsPerPixel == 1 ? PCL6_E_1_BIT : PCL6_E_8_BIT, PCL6_ATTR_COLOR_DEPTH);
    pcl6_write_ubyte(device, PCL6_E_DIRECT_PIXEL, PCL6_ATTR_COLOR_MAPPING);
    pcl6_write_uint16(device, pcl->width, PCL6_ATTR_SOURCE_WIDTH);
    pcl6_write_uint16(device, pcl->height, PCL6_ATTR_SOURCE_HEIGHT);
    pcl6_write_xy(device, pcl->width, pcl->height, PCL6_ATTR_DESTINATION_SIZE);
    pcl6_write_command(device, PCL6_CMD_BEGIN_IMAGE);

    if (header->cupsBitsPerPixel == 1)
      pcl->linesize = (pcl->width + 7) / 8;
    else
      pcl->linesize = pcl->width * header->cupsBitsPerPixel / 8;

    pcl->linesize = (pcl->linesize + 3) & ~3;
    break;
#endif // WITH_PCL6
  }

  // No blank lines yet...
  pcl->feed = 0;

  // Allocate memory for compression...
  if ((pcl->comp_buffer = malloc(pcl->linesize * 2 + 2)) == NULL)
  {
    papplLogJob(job, PAPPL_LOGLEVEL_ERROR, "Memory allocation failure.");
    return (false);
  }

  return (true);
}

//
// 'pcl_rwriteline()' - Write a line.
//

static bool // O - `true` on success, `false` on failure
pcl_rwriteline(
    pappl_job_t *job,            // I - Job
    pappl_pr_options_t *options, // I - Job options
    pappl_device_t *device,      // I - Device
    unsigned y,                  // I - Line number
    const unsigned char *pixels) // I - Line
{
  cups_page_header_t *header = &(options->header);
  // Page header
  pcl_t *pcl = (pcl_t *)papplJobGetData(job);
  // Job data
  unsigned plane,              // Current plane
      x;                       // Current column
  const unsigned char *pixptr; // Pixel pointer in line
  unsigned char bit,           // Current plane data
      *cptr,                   // Pointer into c-plane
      *mptr,                   // Pointer into m-plane
      *yptr,                   // Pointer into y-plane
      *kptr,                   // Pointer into k-plane
      byte;                    // Byte in line
  const unsigned char *dither; // Dither line

  // Skip top and bottom margin areas...
  if (y < pcl->ystart || y >= pcl->yend)
    return (true);

  if (!(y & 127))
    papplLogJob(job, PAPPL_LOGLEVEL_DEBUG, "Printing line %u (%u%%)", y, 100 * (y - pcl->ystart) / pcl->height);

  switch (pcl->driver)
  {
  case HP_DRIVER_DESKJET:
  case HP_DRIVER_GENERIC:
  case HP_DRIVER_LASERJET:
    // Check whether the line is all whitespace...
    byte = options->header.cupsColorSpace == CUPS_CSPACE_K ? 0 : 255;

    if (*pixels != byte || memcmp(pixels, pixels + 1, header->cupsBytesPerLine - 1))
    {
      // No, skip previous whitespace as needed...
      if (pcl->feed > 0)
      {
        papplDevicePrintf(device, "\033*b%dY", pcl->feed);
        pcl->feed = 0;
      }

      // Dither bitmap data...
      dither = options->dither[y & 15];

      if (pcl->num_planes > 1)
      {
        // RGB
        memset(pcl->planes[0], 0, pcl->num_planes * pcl->linesize);

        for (x = pcl->xstart, cptr = pcl->planes[1], mptr = pcl->planes[2], yptr = pcl->planes[3], kptr = pcl->planes[0], pixptr = pixels + 3 * pcl->xstart, bit = 128; x < pcl->xend; x++)
        {
          if (*pixptr++ < dither[x & 15])
            *cptr |= bit;
          if (*pixptr++ < dither[x & 15])
            *mptr |= bit;
          if (*pixptr++ < dither[x & 15])
            *yptr |= bit;

          if (bit == 1)
          {
            *kptr = *cptr & *mptr & *yptr;
            byte = ~*kptr;
            *cptr++ &= byte;
            *mptr++ &= byte;
            *yptr++ &= byte;
            kptr++;
            bit = 128;
          }
          else
            bit /= 2;
        }
      }
      else if (header->cupsBitsPerPixel == 8)
      {
        memset(pcl->planes[0], 0, pcl->linesize);

        if (header->cupsColorSpace == CUPS_CSPACE_K)
        {
          // 8 bit black
          for (x = pcl->xstart, kptr = pcl->planes[0], pixptr = pixels + pcl->xstart, bit = 128, byte = 0; x < pcl->xend; x++, pixptr++)
          {
            if (*pixptr >= dither[x & 15])
              byte |= bit;

            if (bit == 1)
            {
              *kptr++ = byte;
              byte = 0;
              bit = 128;
            }
            else
              bit /= 2;
          }

          if (bit < 128)
            *kptr = byte;
        }
        else
        {
          // 8 bit gray
          for (x = pcl->xstart, kptr = pcl->planes[0], pixptr = pixels + pcl->xstart, bit = 128, byte = 0; x < pcl->xend; x++, pixptr++)
          {
            if (*pixptr < dither[x & 15])
              byte |= bit;

            if (bit == 1)
            {
              *kptr++ = byte;
              byte = 0;
              bit = 128;
            }
            else
              bit /= 2;
          }

          if (bit < 128)
            *kptr = byte;
        }
      }
      else
      {
        // 1-bit B&W
        memcpy(pcl->planes[0], pixels + pcl->xstart / 8, pcl->linesize);
      }

      for (plane = 0; plane < pcl->num_planes; plane++)
        pcl_compress_data(pcl, device, y, pcl->planes[plane], pcl->linesize, plane);
      papplDeviceFlush(device);
    }
    else
      pcl->feed++;
    break;

#if WITH_PCL6
  case HP_DRIVER_GENERIC6:
  case HP_DRIVER_GENERIC6C:
    if (*pixels != byte || memcmp(pixels, pixels + 1, header->cupsBytesPerLine - 1))
      pcl_compress_data(pcl, device, y, pixels + pcl->xstart * header->cupsBitsPerPixel / 8, pcl->linesize, 0);
    break;
#endif // WITH_PCL6
  }

  return (true);
}

//
// 'pcl_status()' - Get printer status.
//

static bool // O - `true` on success, `false` on failure
pcl_status(
    pappl_printer_t *printer) // I - Printer
{
  pappl_device_t *device;    // Printer device
  pappl_supply_t supply[32]; // Printer supply information
  const char *name;          // Printer driver name

  if (papplPrinterGetSupplies(printer, 0, supply) > 0)
  {
    // Already have supplies, just return...
    return (true);
  }

  papplLogPrinter(printer, PAPPL_LOGLEVEL_DEBUG, "Checking status...");

  // First try to query the supply levels via SNMP...
  if ((device = papplPrinterOpenDevice(printer)) != NULL)
  {
    bool success = pcl_update_status(printer, device);

    papplPrinterCloseDevice(printer);

    if (success)
      return (true);
  }

  // Otherwise make sure we have some dummy data to make clients happy...
  name = papplPrinterGetDriverName(printer);

  if (!strcmp(name, "hp_deskjet"))
  {
    static pappl_supply_t inkjet[5] = // Dummy inkjet supply level data
        {
            {PAPPL_SUPPLY_COLOR_CYAN, "Cyan Ink", true, 20, PAPPL_SUPPLY_TYPE_INK},
            {PAPPL_SUPPLY_COLOR_MAGENTA, "Magenta Ink", true, 40, PAPPL_SUPPLY_TYPE_INK},
            {PAPPL_SUPPLY_COLOR_YELLOW, "Yellow Ink", true, 60, PAPPL_SUPPLY_TYPE_INK},
            {PAPPL_SUPPLY_COLOR_BLACK, "Black Ink", true, 80, PAPPL_SUPPLY_TYPE_INK},
            {PAPPL_SUPPLY_COLOR_NO_COLOR, "Waste Ink Tank", true, 50, PAPPL_SUPPLY_TYPE_WASTE_INK}};

    papplPrinterSetSupplies(printer, (int)(sizeof(inkjet) / sizeof(inkjet[0])), inkjet);
  }
  else if (!strcmp(name, "hp_generic6c"))
  {
    static pappl_supply_t claser[4] = // Dummy color laser supply level data
        {
            {PAPPL_SUPPLY_COLOR_CYAN, "Cyan Toner", true, 20, PAPPL_SUPPLY_TYPE_TONER},
            {PAPPL_SUPPLY_COLOR_MAGENTA, "Magenta Toner", true, 40, PAPPL_SUPPLY_TYPE_TONER},
            {PAPPL_SUPPLY_COLOR_YELLOW, "Yellow Toner", true, 60, PAPPL_SUPPLY_TYPE_TONER},
            {PAPPL_SUPPLY_COLOR_BLACK, "Black Toner", true, 80, PAPPL_SUPPLY_TYPE_TONER}};

    papplPrinterSetSupplies(printer, (int)(sizeof(claser) / sizeof(claser[0])), claser);
  }
  else
  {
    static pappl_supply_t laser[1] = // Dummy laser supply level data
        {
            {PAPPL_SUPPLY_COLOR_BLACK, "Black Toner", true, 80, PAPPL_SUPPLY_TYPE_TONER}};

    papplPrinterSetSupplies(printer, (int)(sizeof(laser) / sizeof(laser[0])), laser);
  }

  return (true);
}

//
// 'pcl_update_status()' - Update the supply levels and status.
//

static bool // O - `true` on success, `false` otherwise
pcl_update_status(
    pappl_printer_t *printer, // I - Printer
    pappl_device_t *device)   // I - Device
{
  int num_supply;            // Number of supplies
  pappl_supply_t supply[32]; // Printer supply information
  pappl_preason_t reasons;   // Printer state reasons

  if ((num_supply = papplDeviceGetSupplies(device, (int)(sizeof(supply) / sizeof(supply[0])), supply)) > 0)
    papplPrinterSetSupplies(printer, num_supply, supply);

  papplPrinterSetReasons(printer, papplDeviceGetStatus(device), PAPPL_PREASON_DEVICE_STATUS);

  return (num_supply > 0);
}

#if WITH_PCL6
//
// 'pcl6_write_command()' - Write a command without attributes.
//

static void
pcl6_write_command(
    pappl_device_t *device, // I - Output device
    enum pcl6_cmd command)  // I - Command
{
  unsigned char buffer[1]; // Output buffer

  buffer[0] = (unsigned char)command;
  papplDeviceWrite(device, buffer, 1);
}

//
// 'pcl6_write_data()' - Write a buffer of embedded data.
//

static void
pcl6_write_data(
    pappl_device_t *device,      // I - Output device
    const unsigned char *buffer, // I - Data to write
    size_t length)               // I - Number of bytes
{
  unsigned char length_buf[5]; // Embedded length prefix

  if (length < 0x100)
  {
    // Length < 256 bytes
    length_buf[0] = PCL6_ENC_EMBEDDED_DATA_BYTE;
    length_buf[1] = (unsigned char)length;

    papplDeviceWrite(device, length_buf, 2);
  }
  else
  {
    // Length >= 256 bytes
    length_buf[0] = PCL6_ENC_EMBEDDED_DATA;
    length_buf[1] = (unsigned char)length;
    length_buf[2] = (unsigned char)(length >> 8);
    length_buf[3] = (unsigned char)(length >> 16);
    length_buf[4] = (unsigned char)(length >> 24);

    papplDeviceWrite(device, length_buf, 5);
  }

  papplDeviceWrite(device, buffer, length);
}

#if 0
//
// 'pcl6_write_string()' - Write a single string attribute with optional command.
//

static void
pcl6_write_string(
    pappl_device_t *device,		// I - Output device
    const char     *s,			// I - String
    enum pcl6_attr attr)		// I - Attribute tag
{
  size_t	slen;			// Length of string (max 256 bytes)
  unsigned char	buffer[264],		// Buffer
		*bufptr = buffer;	// Pointer into buffer


  if ((slen = strlen(s)) > 256)
    slen = 256;				// Silently truncate...

  *bufptr++ = PCL6_ENC_UBYTE_ARRAY;
  *bufptr++ = PCL6_ENC_UINT16;
  *bufptr++ = (unsigned char)slen;
  *bufptr++ = (unsigned char)(slen >> 8);

  memcpy(bufptr, s, slen);
  bufptr += slen;

  if (attr < 0x100)
  {
    *bufptr++ = PCL6_ENC_ATTR_UBYTE;
    *bufptr++ = (unsigned char)attr;
  }
  else
  {
    *bufptr++ = PCL6_ENC_ATTR_UINT16;
    *bufptr++ = (unsigned char)attr;
    *bufptr++ = (unsigned char)((unsigned)attr >> 8);
  }

  papplDeviceWrite(device, buffer, (size_t)(bufptr - buffer));
}
#endif // 0

//
// 'pcl6_write_ubyte()' - Write an 8-bit unsigned integer attribute.
//

static void
pcl6_write_ubyte(
    pappl_device_t *device, // I - Output device
    unsigned n,             // I - Number
    enum pcl6_attr attr)    // I - Attribute tag
{
  unsigned char buffer[9], // Buffer
      *bufptr = buffer;    // Pointer into buffer

  *bufptr++ = PCL6_ENC_UBYTE;
  *bufptr++ = (unsigned char)n;

  if (attr < 0x100)
  {
    *bufptr++ = PCL6_ENC_ATTR_UBYTE;
    *bufptr++ = (unsigned char)attr;
  }
  else
  {
    *bufptr++ = PCL6_ENC_ATTR_UINT16;
    *bufptr++ = (unsigned char)attr;
    *bufptr++ = (unsigned char)((unsigned)attr >> 8);
  }

  papplDeviceWrite(device, buffer, (size_t)(bufptr - buffer));
}

//
// 'pcl6_write_uint16()' - Write a 16-bit unsigned integer attribute.
//

static void
pcl6_write_uint16(
    pappl_device_t *device, // I - Output device
    unsigned n,             // I - Number
    enum pcl6_attr attr)    // I - Attribute tag
{
  unsigned char buffer[9], // Buffer
      *bufptr = buffer;    // Pointer into buffer

  *bufptr++ = PCL6_ENC_UINT16;
  *bufptr++ = (unsigned char)n;
  *bufptr++ = (unsigned char)(n >> 8);

  if (attr < 0x100)
  {
    *bufptr++ = PCL6_ENC_ATTR_UBYTE;
    *bufptr++ = (unsigned char)attr;
  }
  else
  {
    *bufptr++ = PCL6_ENC_ATTR_UINT16;
    *bufptr++ = (unsigned char)attr;
    *bufptr++ = (unsigned char)((unsigned)attr >> 8);
  }

  papplDeviceWrite(device, buffer, (size_t)(bufptr - buffer));
}

//
// 'pcl6_write_uint32()' - Write a 32-bit unsigned integer attribute.
//

static void
pcl6_write_uint32(
    pappl_device_t *device, // I - Output device
    unsigned n,             // I - Number
    enum pcl6_attr attr)    // I - Attribute tag
{
  unsigned char buffer[9], // Buffer
      *bufptr = buffer;    // Pointer into buffer

  *bufptr++ = PCL6_ENC_UINT32;
  *bufptr++ = (unsigned char)n;
  *bufptr++ = (unsigned char)(n >> 8);
  *bufptr++ = (unsigned char)(n >> 16);
  *bufptr++ = (unsigned char)(n >> 24);

  if (attr < 0x100)
  {
    *bufptr++ = PCL6_ENC_ATTR_UBYTE;
    *bufptr++ = (unsigned char)attr;
  }
  else
  {
    *bufptr++ = PCL6_ENC_ATTR_UINT16;
    *bufptr++ = (unsigned char)attr;
    *bufptr++ = (unsigned char)((unsigned)attr >> 8);
  }

  papplDeviceWrite(device, buffer, (size_t)(bufptr - buffer));
}

//
// 'pcl6_write_xy()' - Write a single X,Y attribute.
//

static void
pcl6_write_xy(
    pappl_device_t *device, // I - Output device
    unsigned x,             // I - X coordinate
    unsigned y,             // I - Y coordinate
    enum pcl6_attr attr)    // I - Attribute tag
{
  unsigned char buffer[13], // Buffer
      *bufptr = buffer;     // Pointer into buffer

  if (x < 0x100 && y < 0x100)
  {
    *bufptr++ = PCL6_ENC_UBYTE_XY;
    *bufptr++ = (unsigned char)x;
    *bufptr++ = (unsigned char)y;
  }
  else if (x < 0x10000 && y < 0x10000)
  {
    *bufptr++ = PCL6_ENC_UINT16_XY;
    *bufptr++ = (unsigned char)x;
    *bufptr++ = (unsigned char)(x >> 8);
    *bufptr++ = (unsigned char)y;
    *bufptr++ = (unsigned char)(y >> 8);
  }
  else
  {
    *bufptr++ = PCL6_ENC_UINT32_XY;
    *bufptr++ = (unsigned char)x;
    *bufptr++ = (unsigned char)(x >> 8);
    *bufptr++ = (unsigned char)(x >> 16);
    *bufptr++ = (unsigned char)(x >> 24);
    *bufptr++ = (unsigned char)y;
    *bufptr++ = (unsigned char)(y >> 8);
    *bufptr++ = (unsigned char)(y >> 16);
    *bufptr++ = (unsigned char)(y >> 24);
  }

  if (attr < 0x100)
  {
    *bufptr++ = PCL6_ENC_ATTR_UBYTE;
    *bufptr++ = (unsigned char)attr;
  }
  else
  {
    *bufptr++ = PCL6_ENC_ATTR_UINT16;
    *bufptr++ = (unsigned char)attr;
    *bufptr++ = (unsigned char)((unsigned)attr >> 8);
  }

  papplDeviceWrite(device, buffer, (size_t)(bufptr - buffer));
}
#endif // WITH_PCL6
