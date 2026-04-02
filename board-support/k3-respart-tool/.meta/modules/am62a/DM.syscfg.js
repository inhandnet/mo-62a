
const {createHostModule} = system.getScript("/.meta/modules/sysfwResPart.js");
const hostInfo = {
  "Description": "Device Management",
  "Security": "Non Secure",
  "displayName": "Device Management",
  "hostId": 254,
  "hostName": "DM"
};
const modDef = createHostModule(hostInfo);
exports = modDef;
