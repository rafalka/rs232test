

# Introduction #
RS232Test is simple serial terminal that allows entering and displaying received data either in text or binary (HEX) mode.

# Command Line options #

| `-h, --help`  | Show command line options |
|:--------------|:--------------------------|
| `-c, --connect`  | Automatically connect to port after starting |
| `--config=cfgfile`  | Configuration file. If not defined then Registry will be used |
| `-p, --port=port name`  | Port file name that should be selected |
| `-l, --log[=log file]`  | Dump output to log file (in HTML format) with provided name or default file name if [file](log.md) parameter is not provided.

&lt;BR&gt;

Auto log file name format:

&lt;BR&gt;

`Rs232test2_yyyy-MM-dd_hh.mm.ss.zzz.log.html` |
| `-i, --inmode=mode`  | Edit mode. 

&lt;BR&gt;

 Possible `<mode>` values:

&lt;BR&gt;

`hex|h` - HEX 

&lt;BR&gt;

`ascii|a` - ASCII 

&lt;BR&gt;

`cstr|c` - C-like string |
| `-o, --outmode=mode`  | Display mode. 

&lt;BR&gt;

Possible `<mode>` values:

&lt;BR&gt;

`hex|h` - HEX 

&lt;BR&gt;

`ascii|a` - ASCII 

&lt;BR&gt;

`cstr|c` - C-like string |