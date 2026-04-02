
const {createHostModule} = system.getScript("/.meta/modules/sysfwResPart.js");
const hostInfo = {
  "Description": "DM to TIFS communication",
  "Security": "Secure",
  "displayName": "DM to TIFS communication",
  "hostId": 250,
  "hostName": "DM2TIFS"
};
const modDef = createHostModule(hostInfo);
exports = modDef;
