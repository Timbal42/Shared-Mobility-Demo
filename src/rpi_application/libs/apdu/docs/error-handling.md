## Error Handling

The library relies on the [ifx-error](https://bitbucket.vih.infineon.com/projects/V2XSYS/repos/hsw-ifx-error/browse) library to encode its return values.

It is uses the `LIBAPDU` module identifier (`0x10`) for all errors.

The custom defined function identifiers with their custom error reasons are:

* `0x01`: `APDU_DECODE`
    Function identifier for `apdu_decode(APDU*, uint8_t*, size_t)` with custom error reasons:
    * `0x01`: `LC_MISMATCH`
        Error reason if APDU's `LC` does not match length of the given data.
    * `0x02`: `EXTENDED_LENGTH_MISMATCH`
        Error reason if LC and LE do not use same form (short / extended)

* `0x02`: `APDU_ENCODE`
    Function identifier for `apdu_encode(APDU*, uint8_t**, size_t*)` with no custom error reasons.

* `0x03`: `APDURESPONSE_DECODE`
    Function identifier for `apduresponse_decode(APDUResponse*, uint8_t*, size_t)` with no custom error reasons.

* `0x04`: `APDURESPONSE_ENCODE`
    Function identifier for `apduresponse_encode(APDUResponse*, uint8_t**, size_t*)` with no custom error reasons.