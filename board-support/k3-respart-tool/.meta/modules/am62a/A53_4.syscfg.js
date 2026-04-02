
const {createHostModule} = system.getScript("/.meta/modules/sysfwResPart.js");
const hostInfo = {
  "Description": "Cortex A53 context 4 on Main island",
  "Security": "Non Secure",
  "displayName": "A53_4 NonSecure host",
  "hostId": 14,
  "hostName": "A53_4",
  "privId": 1
};
const modDef = createHostModule(hostInfo);
exports = modDef;
