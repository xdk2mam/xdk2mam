const ttn = require("ttn")
const { appID, accessKey, provider } = require('./config.json')
const { convert2Xdk2mamFormat, publish } = require('./xdk2mamUtils')


ttn.data(appID, accessKey)
    .then(client => {
        client.on("uplink", async (devID, payload) => {
            if (payload.payload_fields) {

                const payload_fields = payload.payload_fields
                const timestamp = new Date(payload.metadata.time).getTime()

                const data = convert2Xdk2mamFormat(payload_fields)

                await publish(
                    { xdk2mam: data, device: devID, timestamp },
                    provider
                )


            }
        })
    })
    .catch(error => {
        console.error("Error", error)

    })

