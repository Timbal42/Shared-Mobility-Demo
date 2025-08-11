## Error Handling

The library relies on the [ifx-error](https://bitbucket.vih.infineon.com/projects/GOLDENEYE_EVALBOARD/repos/hsw-ifx-error/browse) library to encode its return values.

It is uses the `LIBI2C` module identifier (`0x23`) for all errors.

At the moment no custom function identifiers and error reasons are required.