## Error Handling

The library relies on the [ifx-error](https://bitbucket.vih.infineon.com/projects/V2XSYS/repos/hsw-ifx-error/browse) library to encode its return values.

It is uses the `LIBPROTOCOL` module identifier (`0x20`) for all errors.

The reusable error reasons are:

* `0x8f`: `INVALID_PROTOCOLSTACK`
    Used by all functions if the protocol struct is corrupted (required function missing)


The reusable function identifiers are:

* `0x81`: `PROTOCOL_ACTIVATE`
    Function identifier for `protocol_activate(Protocol*, uint8_t**, size_t*)` and all other implementations of `protocol_activatefunction_t` with no custom error reasons.

* `0x82`: `PROTOCOL_TRANSCEIVE`
    Function identifier for `protocol_transceive(Protocol*, uint8_t*, size_t, uint8_t**, size_t*)` and all other implementations of `protocol_transceivefunction_t` with no custom error reasons.

* `0x83`: `PROTOCOL_TRANSMIT`
    Function identifier for all functions sending data to secure element.

* `0x84`: `PROTOCOL_RECEIVE`
    Function identifier for all functions receiving data from secure element

* `0x85`: `PROTOCOL_GETPROPERTY`
    Function identifier for all functions querying protocol parameters

* `0x86`: `PROTOCOL_SETPROPERTY`
    Function identifier for all functions setting protocol parameters

* `0x87`: `PROTOCOLLAYER_INITIALIZE`
    Function identifier for all functions initializing a protocol layer
