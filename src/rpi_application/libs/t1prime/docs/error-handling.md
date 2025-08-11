## Error Handling

The library relies on the [ifx-error](https://bitbucket.vih.infineon.com/projects/V2XSYS/repos/hsw-ifx-error/browse) library to encode its return values.

It is uses the `LIBT1PRIME` module identifier (`0x21`) for all errors.

The reusable error reasons are:

* `0x60`: `TRANSCEIVE_ABORTED`
    Used by `protocol_tranceive(Protocol*, uint8_t*, size_t, uint8_t**, size_t*)` if secure element aborted chained communication
* `0x61`: `INVALID_BLOCK`
    Used by all functions interacting with secure element if an invalid block has been received

The custom defined function identifiers with their custom error reasons are:

* `0x30`: `T1PRIME_CIP_DECODE`,
* `0x31`: `T1PRIME_CIP_VALIDATE`,
* `0x32`: `T1PRIME_DLLP_DECODE`,
* `0x33`: `T1PRIME_PLP_DECODE`,
* `0x34`: `T1PRIME_IFS_DECODE`, and
* `0x35`: `T1PRIME_IFS_ENCODE`
    Function identifiers for private data validation functions
    `t1prime_cip_decode(CIP*, uint8_t*, size_t)`,
    `t1prime_cip_validate(CIP*, uint8_t*, size_t)`,
    `t1prime_dllp_decode(DLLP*, uint8_t*, size_t)`,
    `t1prime_spi_plp_decode(SPIPLP*, uint8_t*, size_t)`,
    `t1prime_i2c_plp_decode(I2CPLP*, uint8_t*, size_t)`,
    `t1prime_ifs_decode(size_t*, uint8_t*, size_t)` and
    `t1prime_ifs_encode(size_t, uint8_t**, size_t*)`
     with custom error reasons:
    * `0x01`: `INVALID_LENGTH`
        Length information does not match data length
    * `0x02`: `INVALID_PLID`
        Invalid physical layer ID given
