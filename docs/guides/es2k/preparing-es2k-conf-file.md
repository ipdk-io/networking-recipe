# Preparing the ES2K Configuration File

This document demonstrates how to customize the `es2k_skip_p4.conf`
configuration file for your P4 program.

In this case, we will be modifying it for the `simple_l3_l4_pna` sample program.

> **Notes:**
> 1. Should we mention that the user has the option of saving the
> edited file under a different name and using the `-es2k_infrap4d_cfg`
> command-line option to tell `infrap4d` to load it instead of the default
> file?
> 2. `es2k_skip_p4` is an indication (carried over from Tofino) of what
> kind of configuration this is. Is "skip_p4" really the best name? Wouldn't
> a better name for the standard config file be `esk2_infrap4d.conf`? The
> idea is for the customer to use the template to create a program-specific
> configuration file from a template and save it under a name that reflects
> the program it runs, then either specify the program-specific config file
> on the `infrap4d` command line or copy it to `es2k_infrap4d.conf`.
>
> It appears to me that we've made things confusing by not thinking about
> the customer here.

## Sample File

/usr/share/stratum/es2k/es2k_skip_p4.conf

> **Suggestions:**
> 1. Replace *all* parameter values to be customized with uppercase placeholders
     (e.g. `PCIE-BDF`).
> 2. Change ABSOLUTE-PATH to FULL-PATH.
> 3. Better still, shorten the placeholder names by omitting the
     ABSOLUTE-PATH-TO- prefix entirely (CONTEXT-JSON-FILE, JSON-FILES-PATH).
     The shorter names are easier to read and understand, and they alphabetize
     better.

```json
{
    "chip_list": [
    {
        "id": "asic-0",
        "chip_family": "mev",
        "instance": 0,
        "pcie_bdf": "0000:af:00.6",
        "iommu_grp_num": 152
    }
    ],
    "instance": 0,
    "cfgqs-idx": "0-15",
    "p4_devices": [
    {
        "device-id": 0,
        "fixed_functions" : [],        
        "eal-args": "--lcores=1-2 -a af:00.6,vport=[0-1] -- -i --rxq=1 --txq=1 --hairpinq=1 --hairpin-mode=0x0",
        "p4_programs": [
        {
            "program-name": "P4-PROGRAM-NAME",
            "bfrt-config": "ABSOLUTE-PATH-TO-BFRT-JSON-FILE",
            "p4_pipelines": [
            {
                "p4_pipeline_name": "P4-PIPELINE-NAME",
                "context": "ABSOLUTE-PATH-TO-CONTEXT-JSON-FILE",
                "config": "ABSOLUTE-PATH-TO-TOFINO-BIN-FILE",
                "pipe_scope": [
                    0,
                    1,
                    2,
                    3
                ],
                "path": "ABSOLUTE-PATH-FOR-JSON-FILES"
            }
            ]
        }
        ],
        "agent0": "lib/libpltfm_mgr.so"
    }
    ]
}
```

## Parameters

> **Suggestions:**
> 1. Use placeholders (`PCIE-BDF`) instead of parameter names (`pcie-bdf`)
     as headings.
> 2. List parameters in alphabetical order.

Handcraft the configuration file `/usr/share/stratum/es2k/es2k_skip_p4.conf`
with the following parameters:

### `pcie_bdf`

Get PCI BDF of LAN Control Physical Function (CPF) device with device
ID 1453 on ACC.

```bash
lspci | grep 1453
00:01.6 Ethernet controller: Intel Corporation Device 1453 (rev 11)
```

The value of `pcie_bdf` should be `00:01.6`.

### `iommu_grp_num`

To determine the iommu group number:

```bash
cd $SDE_INSTALL/bin/

./vfio_bind.sh 8086:1453
Device: 0000:00:01.6 Group = 5
```

The value of `iommu_grp_num` should be `5`.

### `vport`

The number of vports supported is from [0-6].
For example: `vport=[0-1]`.

### `eal_args`

Supported values for `--proc-type` are `primary` and `auto`.

In case of multi-process setup which is supported in docker
environment, `--proc-type` can be used to specify the process type.

### `cfgqs-idx`

We give options to each process (primary or secondary) to request
the number of configure queues. Admin must set cfgqs-idx between `"0-15"`,
recommended option when running only one process.

In multi-process mode, plan and divide config queues between processes.
For example, to configure two queues, specify `cfgqs-idx: "0-1"`.

Supported index numbers are from 0 to 15.

### `program-name`

Specify the name of the P4 program. For simple_l3_l4_pna, replace
`P4-PROGRAM-NAME` with `simple_l3_l4_pna`.

### `p4_pipeline_name`

Specify the name of P4 pipeline. For simple_l3_l4_pna, replace
`P4-PIPELINE-NAME` with `main`.

### `bfrt-config`

Specify the full path to the `bfrt.json` file.

Replace `ABSOLUTE-PATH-TO-BFRT-JSON-FILE` with
`/opt/p4/p4sde/share/mev_reference_p4_files/simple_l3_l4_pna/simple_l3_l4_pna.bf-rt.json`.

### `context`

Specify the full path to the `context.json` file.

Replace `ABSOLUTE-PATH-TO-CONTEXT-JSON-FILE` with.
`/opt/p4/p4sde/share/mev_reference_p4_files/simple_l3_l4_pna/simple_l3_l4_pna.context.json`.

### `config`

Specify the full path to the `tofino.bin` file.

Replace `ABSOLUTE-PATH-TO-TOFINO-BIN-FILE` with
`/opt/p4/p4sde/share/mev_reference_p4_files/simple_l3_l4_pna/tofino.bin`.

### `path`

Specify the full path to the directory containing the json files.

> **Question:** *which* json files?

Replace `ABSOLUTE-PATH-FOR-JSON-FILES` with
`/opt/p4/p4sde/share/mev_reference_p4_files/simple_l3_l4_pna`.

## Finished Result

The final es2k_skip_p4.conf for the simple_l3_l4_pna sample program will look
like this:

```json
{
    "chip_list": [
    {
        "id": "asic-0",
        "chip_family": "mev",
        "instance": 0,
        "pcie_bdf": "0000:00:01.6",
        "iommu_grp_num": 5
    }
    ],
    "instance": 0,
    "cfgqs-idx": "0-15",
    "p4_devices": [
    {
        "device-id": 0,
        "fixed_functions" : [],
        "eal-args": "--lcores=1-2 -a 00:01.6,vport=[0-1] -- -i --rxq=1 --txq=1 --hairpinq=1 --hairpin-mode=0x0",
        "p4_programs": [
        {
            "program-name": "simple_l3_l4_pna",
            "bfrt-config": "/opt/p4/p4sde/share/mev_reference_p4_files/simple_l3_l4_pna/simple_l3_l4_pna.bf-rt.json",
            "p4_pipelines": [
            {
                "p4_pipeline_name": "main",
                "context": "/opt/p4/p4sde/share/mev_reference_p4_files/simple_l3_l4_pna/simple_l3_l4_pna.context.json",
                "config": "/opt/p4/p4sde/share/mev_reference_p4_files/simple_l3_l4_pna/tofino.bin",
                "pipe_scope": [
                    0,
                    1,
                    2,
                    3
                ],
                "path": "/opt/p4/p4sde/share/mev_reference_p4_files/simple_l3_l4_pna"
            }
            ]
        }
        ],
        "agent0": "lib/libpltfm_mgr.so"
    }
    ]
}
```
