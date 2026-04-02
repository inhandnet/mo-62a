
const {createHostModule} = system.getScript("/.meta/modules/sysfwResPart.js");
const hostInfo = {
  "Description": "MCU R5",
  "Security": "Non Secure",
  "displayName": "MCU R5F0 core0 NonSecure host",
  "hostId": 30,
  "hostName": "MCU_0_R5_0",
  "privId": 96
};
const modDef = createHostModule(hostInfo);
exports = modDef;
