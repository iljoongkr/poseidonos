## poseidonos-cli array rmspare

Remove a spare device from an array of PoseidonOS.

### Synopsis


Remove a spare device from an array of PoseidonOS.

Syntax:
	poseidonos-cli array rmspare (--spare | -s) DeviceName (--array-name | -a) ArrayName

Example: 
	poseidonos-cli array rmspare --spare SpareDeviceName --array-name Array0
          

```
poseidonos-cli array rmspare [flags]
```

### Options

```
  -a, --array-name string   The name of the array to remove the specified spare device.
  -h, --help                help for rmspare
  -s, --spare string        The name of the device to remove from the array.
```

### Options inherited from parent commands

```
      --debug         Print response for debug.
      --fs string     Field separator for the output. (default "|")
      --ip string     Set IPv4 address to PoseidonOS for this command. (default "127.0.0.1")
      --json-req      Print request in JSON form.
      --json-res      Print response in JSON form.
      --port string   Set the port number to PoseidonOS for this command. (default "18716")
      --unit          Display unit (B, KB, MB, ...) when displaying capacity.
```

### SEE ALSO

* [poseidonos-cli array](poseidonos-cli_array.md)	 - Array command for PoseidonOS.

