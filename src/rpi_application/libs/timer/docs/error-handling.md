## Error Handling

The library relies on the [ifx-error](https://bitbucket.vih.infineon.com/projects/V2XSYS/repos/hsw-ifx-error/browse) library to encode its return values.

It is uses the `LIBTIMER` module identifier (`0x02`) for all errors.

The custom defined function identifiers with their custom error reasons are:

* `0x01`: `TIMER_SET`
    Function identifier for `timer_set(Timer*, uint64_t)` with no custom error reasons.
* `0x02`: `TIMER_JOIN`
    Function identifier for `timer_join(Timer*)` with custom error reasons:
    * `0x01`: `TIMER_NOT_SET`
        Error reason if `timer_join(Timer*)` has been called on a `Timer` that has not been set.
