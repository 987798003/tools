
# ix-sn-tool

**ix-sn-tool** is a tool helps to online burning board sn.

### 1. Usage

---

The syntax of ix-sn-tool is the following(args are optional options for ix-sn-tool):

```
./ix-sn-tool [args] [argv]
```
- `-d, --device`：Specify device bus id.
- `-f, --file`：Specify the Bin file to be burned.
- `--sn, --snserial`：Specify sn sequence number and generate sn file.
- `--pn, --pnserial`：Specify pn sequence number.
- `--date`：Specify board production time(Need to be used with --sn).

```
./ix-sn-tool [args]
```
- `-l,--list`：List all device information.
- `-h,--help`：Print this help message and exit.


### 2. How to use ix-sn-tool

---

Assuming the `ix-sn-tool` online burning board sn tool stored in a `~/sw_home/tools/ixToolbox/build`, The following are two examples of the use of `ix-sn-tool`:

1. View the bdf, device name, and sn serial number of the current device.
   ```
   ./ix-sn-tool -l
   ```

2. Specify the device to be burned into sn online through the bin file (the first board is burned by default, which is applicable when there is only one board).
   ```
   ./ix-sn-tool -f file.bin
   ```

3. Specify the device to burn sn online through bin file.
   ```
   ./ix-sn-tool -d BDF -f file.bin
   ```

4. Specify the device to generate the SN file and burn it online, and specify the GPU production time (default is the current time).
   ```
   ./ix-sn-tool -d BDF --sn snSerial --date date
   ```

5. Specify the device that generates the SN file and burns it online, and specify the GPU production time and the corresponding PN code (the default is the current time and PN code)
   ```
   ./ix-sn-tool -d BDF --sn snSerial --date date --pn pnSerial
   ```

### 3. Features

---

- Support online burning sn through bin file.
- Support generating sn file and burning it online.
- Support specifying the device to burn sn online.
- Support specifying the GPU production time.
- Supports manual specification of different types of PN codes.
- Support default upgrade of the first board,which is applicable when there is only one board.
