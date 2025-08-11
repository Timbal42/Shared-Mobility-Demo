## Error Handling

The library relies on the [ifx-error](https://bitbucket.vih.infineon.com/projects/GOLDENEYE_EVALBOARD/repos/hsw-ifx-error/browse) library to encode its return values.

It is uses the `LIBAARDVARKI2C` module identifier (`0x34`) for all errors.

The custom defined function identifiers with their custom error reasons are:

* `0x81`: `PROTOCOL_ACTIVATE`
    Reuse of function identifier for `protocol_activate(Protocol*, uint8_t**, size_t*)` with new custom error reasons:
    * `0xaa`: `NO_READER_FOUND`
        Error reason if no Aardvark I2C reader was found when trying to establish a connection.
    * `0xab`: `DEVICE_BUSY`
        Error reason if Aardvark I2C reader is blocked when trying to establish a connection.
    * `0xac`: `CANNOT_CONNECT_TO_READER`
        Error reason if no connection to Aardvark I2C reader could be established.

* `0x01`: `AARDVARK_LOAD_DLL`
    Function identifier for private function `aardvark_load_dll()` with no custom error reasons.