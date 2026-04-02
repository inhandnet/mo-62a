const deviceSelected = system.deviceData.device;
const devData = _.keyBy(system.getScript("/.meta/data/SOC.json"), (r) => r.soc);
const socName = devData[deviceSelected].shortName;
const pdkUsage = devData[deviceSelected].pdkUsage;
const mcusdkUsage = devData[deviceSelected].mcusdkUsage;
var hosts = system.getScript("/.meta/data/" + socName + "/Hosts.json");

exports = {
	displayName: "Resource Partitioning",
	templates: [
		{
			name: "/.meta/templates/rm-cfg.syscfg.xdt",
			outputPath: "rm-cfg.c",
			alwaysRun: true,
		},
		{
			name: "/.meta/templates/sciclient_defaultBoardcfg_rm.syscfg.xdt",
			outputPath: "sciclient_defaultBoardcfg_rm.c",
			alwaysRun: pdkUsage,
		},
		{
			name: "/.meta/templates/sciclient_defaultBoardcfg_rm_mcusdk.syscfg.xdt",
			outputPath: "sciclient_defaultBoardcfg_rm_mcusdk.c",
			alwaysRun: mcusdkUsage,
		},
		{
			name: "/.meta/templates/sciclient_defaultBoardcfg_tifs_rm.syscfg.xdt",
			outputPath: "sciclient_defaultBoardcfg_tifs_rm.c",
			alwaysRun: pdkUsage,
		},
		{
			name: "/.meta/templates/tifs-rm-cfg.syscfg.xdt",
			outputPath: "tifs-rm-cfg.c",
			alwaysRun: true,
		},
		{
			name: "/.meta/templates/bwlimiters_header.syscfg.xdt",
			outputPath: socName + "_bwl.h",
			alwaysRun: true,
		},
		{
			name: "/.meta/templates/qos_header.syscfg.xdt",
			outputPath: socName + "_qos.h",
			alwaysRun: true,
		},
		{
			name: "/.meta/templates/qos_config_uboot.syscfg.xdt",
			outputPath: socName + "_qos_uboot.c",
			alwaysRun: true,
		},
		{
			name: "/.meta/templates/qos_config.syscfg.xdt",
			outputPath: socName + "_qos_data.c",
			alwaysRun: true,
		},
		{
			name: "/.meta/templates/firewall_config.syscfg.xdt",
			outputPath: socName + "_firewall_data.c",
			alwaysRun: true,
		},
		{
			name: "/.meta/templates/sciclient_defaultBoardcfg.syscfg.xdt",
			outputPath: "sciclient_defaultBoardcfg.c",
			alwaysRun: true,
		},
		{
			name: "/.meta/templates/sysfw_img_cfg.syscfg.xdt",
			outputPath: "sysfw_img_cfg.h",
			alwaysRun: true,
		},
		{
			name: "/.meta/templates/rm-cfg-yaml.syscfg.xdt",
			outputPath: "rm-cfg.yaml",
			alwaysRun: true,
		},
		{
			name: "/.meta/templates/tifs-rm-cfg-yaml.syscfg.xdt",
			outputPath: "tifs-rm-cfg.yaml",
			alwaysRun: true,
		},
	],
	topModules: [
		{
			displayName: "SYSFW Board config",
			modules: ["/.meta/modules/boardConfig"],
		},
		{
			displayName: "SYSFW Resource Partitioning",
			modules: get_host_modules(),
		},
		{
			displayName: "Resource Sharing",
			modules: ["/.meta/modules/resourceSharing"],
		},
		{
			displayName: "Peripheral Resource Partitioning",
			modules: ["/.meta/modules/qosConfig", "/.meta/modules/firewallConfig", "/.meta/modules/bwlimitersConfig"],
		}
	],
	views: [
		{
			name: "/.meta/templates/resAllocMarkdown.xdt",
			displayName: "Resource Allocation Markdown",
			viewType: "markdown",
			ignoreErrors: true,
		},
		{
			name: "/.meta/templates/resAllocTable.xdt",
			displayName: "Resource Allocation Table",
			viewType: "markdown",
			ignoreErrors: true,
		},
		{
			name: "/.meta/templates/hwaTable.xdt",
			displayName: "HWA Channels Table",
			viewType: "markdown",
			ignoreErrors: true,
		},
	],
};

function get_host_modules() {
	var modules = [];
	for (var idx = 0; idx < hosts.length; idx++) {
		modules.push("/.meta/modules/" + socName + "/" + hosts[idx].hostName);
	}
	return modules;
}
