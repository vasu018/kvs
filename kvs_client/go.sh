sudo -E gdb --args ./kvs_client/$(printenv RTE_TARGET)/app/kvs_client --proc-type=secondary  --file-prefix kvs
