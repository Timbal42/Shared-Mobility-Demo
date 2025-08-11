## Error Handling

The library relies on the [ifx-error](https://bitbucket.vih.infineon.com/projects/V2XSYS/repos/hsw-ifx-error/browse) library to encode its return values.

It is uses the `LIBLOGGER` module identifier (`0x50`) for all errors.

The reusable function identifiers are:

* `0x90`: `LOGGER_INITIALIZE`
    Function identifier for any function initializing a Logger with no custom error reasons

* `0x91`: `LOGGER_LOG`
    Function identifier for any function logging data with custom error reason:
      * `0x01`: `FORMAT_ERROR`
          Error reason if error occured formatting string

The custom defined function identifiers with their custom error reasons are:

* `0x01`: `LOGGER_SET_LEVEL`
    Function identifier for `logger_set_level(Logger*, LogLevel)` with no custom error reasons
