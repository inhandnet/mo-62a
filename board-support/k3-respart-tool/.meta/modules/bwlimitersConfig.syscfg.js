const utils = system.getScript("/.meta/scripts/utils.js");

const deviceSelected = utils.socName;
const devData = utils.devData;
const socName = utils.socName;
const bwlimiter = utils.bwlimiter;

function getDeviceNameOptions() {
	var deviceNames = [];
	_.each(bwlimiter, (d) => {
		deviceNames.push(d.deviceName);
	});
	deviceNames = _.uniq(deviceNames);

	var deviceOpt = _.map(deviceNames, (d) => {
		return {
			name: d,
			displayName: d,
		};
	});
	deviceOpt.unshift({
		name: "unknown",
		displayName: "Select",
	});

	return deviceOpt;
}

var documentation = `
**Bandwidth Limiters configuration**

---

This module allows you to configure the rate limiters for the CBA 4.0 vbusm
interface. Here you will only find the rate limiters available for the selected
SOC. Limits can be imposed on the bandwidth consumed as well as the maximum
outstanding transactions, for read or write operations from various devices on
the SOC. These configurations are all orthogonal, so the bandwidth limits can
be configured independent of the transaction limits.

Following steps allow you to achieve this:

*	Click on ADD button to program a rate limiter
*	Select the device from the drop down list of available rate limiters
*	Choose to enable bandwidth and transaction limits as required
*	Type in the appropriate values for the various limits available

**Bandwidth Calculation Details**

The Committed Information Rate (CIR) and Peak Information Rate (PIR) register values
are effectively a bandwidth measured in bytes per cycle, where the bits 31:18 are
whole bytes per cycle and 17:0 are the right hand side of a decimal point - i.e.
representing some fraction of a byte per cycle. A value of 32'h40000, with bit 18
alone being 1, therefore represents a single byte per cycle of bandwidth.

The conversion formula for the policing rate above is to translate a target
bandwidth from Mbit/s into the bytes per cycle equivalent. As an example, if one
wished to program a bandwidth limit of 200 MB/s (1600 Mbps) at a frequency of
400 MHz, that would work out as:
-    1600 = 400 * CIR_reg / 32768
-    CIR_reg = 1600/400*32768
-    CIR_reg = 131071 = 32'h20000

To allow additional initial bandwidth for IP startup, the bandwidth limiter supports
a burst offset. This is an offset to the CIR counter to allow for a configurable
number of bytes of traffic on the bus before enforcing the commit rate (CIR). The
peak rate (PIR) will still be enforced.


**Output files**

---

*	\`qos_data.c\` - This file should be copied to the bootloader for
configuring the registers. This file has a simple address, value
pairs in an array. The bootloader is supposed to iterate over the
array and program the registers one-time as part of the bootup.


`;

exports = {
	displayName: "BW Limiters",
	longDescription: documentation,
	config: [
		{
			name: "deviceName",
			displayName: "Rate limiter device",
			options: getDeviceNameOptions(),
			default: "unknown",
			onChange: (inst, ui) => {
				if (inst.deviceName === "unknown") {
					ui.bw_enable.hidden = true;
					ui.limitCIR.hidden = true;
					ui.limitPIR.hidden = true;
					ui.limitBurst.hidden = true;
					ui.txn_enable.hidden = true;
					ui.limitTxn.hidden = true;
				} else {
					ui.bw_enable.hidden = false;
					ui.limitCIR.hidden = false;
					ui.limitPIR.hidden = false;
					ui.limitBurst.hidden = false;
					ui.txn_enable.hidden = false;
					ui.limitTxn.hidden = false;
				}
			},
		},
		{
			name: "bw_enable",
			displayName: "Enable Bandwidth Limiting",
			default: false,
			hidden: true,
			onChange: (inst, ui) => {
				if (inst.bw_enable) {
					ui.limitCIR.readOnly = false;
					ui.limitPIR.readOnly = false;
					ui.limitBurst.readOnly = false;
				} else {
					inst.limitCIR = inst.defaultCIR;
					ui.limitCIR.readOnly = true;
					inst.limitPIR = inst.defaultPIR;
					ui.limitPIR.readOnly = true;
					inst.limitBurst = inst.defaultBurst;
					ui.limitBurst.readOnly = true;
				}
			},
		},
		{
			name: "limitCIR",
			displayName: "BW Limit for Committed Information Rate (CIR)",
			default: "0",
			hidden: true,
			readOnly: true,
		},
		{
			name: "defaultCIR",
			default: "0",
			hidden: true,
		},
		{
			name: "limitPIR",
			displayName: "BW Limit for Peak Information Rate (PIR)",
			default: "0",
			hidden: true,
			readOnly: true,
		},
		{
			name: "defaultPIR",
			default: "0",
			hidden: true,
		},
		{
			name: "limitBurst",
			displayName: "Burst offset before the CIR is applied",
			default: "0",
			hidden: true,
			readOnly: true,
		},
		{
			name: "defaultBurst",
			default: "0",
			hidden: true,
		},
		{
			name: "txn_enable",
			displayName: "Enable limiting maximum outstanding transactions",
			default: false,
			hidden: true,
			onChange: (inst, ui) => {
				if (inst.txn_enable) {
					ui.limitTxn.readOnly = false;
				} else {
					inst.limitTxn = inst.defaultTxn;
					ui.limitTxn.readOnly = true;
				}
			},
		},
		{
			name: "limitTxn",
			displayName: "Limit on the maximum outstanding transactions",
			default: "0x40",
			hidden: true,
			readOnly: true,
		},
		{
			name: "defaultTxn",
			default: "0x40",
			hidden: true,
		},
	],
	validate: (inst, report) => {
		if (inst.deviceName == "unknown") {
			report.logError("Select a device from the list", inst, "deviceName");
		} else {
			check_overlapped_deviceNames(inst, report);
		}
	},
};


function check_overlapped_deviceNames(inst, report) {
	var moduleInstance = inst.$module.$instances;

	_.each(moduleInstance, (i) => {
		if (i !== inst) {
			if (i.deviceName === inst.deviceName) {
				report.logError("This rate limiter Device is used more than once" +
					"(in " + i.$name + ")", inst, "deviceName");
			}
		}
	});
}
