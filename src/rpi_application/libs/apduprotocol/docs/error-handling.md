## Error Handling

The library relies on the [ifx-error](https://bitbucket.vih.infineon.com/projects/V2XSYS/repos/hsw-ifx-error/browse) library to encode its return values.

It is uses the `LIBAPDUPROTOCOL` module identifier (`0x28`) for all errors.

The reusable error reasons are:

* `0xb0`: `STATUS_WORD_ERROR`
    Can be used by functions in other libraries to indicate an invalid status word has been received as part of the APDU response.
