const Mam = require('@iota/mam')
const { asciiToTrytes } = require('@iota/converter')
const colors = require('colors')

const publish = async (payload, provider) => {

    let mamState = Mam.init(provider)
    const trytes = asciiToTrytes(JSON.stringify(payload))
    const message = Mam.create(mamState, trytes)
    mamState = message.state
    const root = message.root
    await Mam.attach(message.payload, message.address)

    console.log('Payload:', JSON.stringify(payload))
    console.log("Root:", colors.bold.green(root))
    console.log('*************************************************************')
}

const convert2Xdk2mamFormat = value => {
    var xdk2mam = []
    var sensor

    if (value.temperature_1 || value.relative_humidity_2 || value.barometric_pressure_3) {
        sensor = 'Environmental'
        var data = []
        data.push({
            Pressure: value.barometric_pressure_3
        })
        data.push({
            Temp: value.temperature_1 * 1000
        })
        data.push({
            Humidity: value.relative_humidity_2
        })
        xdk2mam.push({ sensor, data })
    }
    if (value.luminosity_4) {
        sensor = 'Light'
        var data = []
        data.push({
            milliLux: value.luminosity_4
        })

        xdk2mam.push({ sensor, data })
    }
    if (value.accelerometer_5) {
        sensor = 'Accel'
        var data = []
        data.push({
            x: value.accelerometer_5.x.toString()
        })
        data.push({
            y: value.accelerometer_5.y.toString()
        })
        data.push({
            z: value.accelerometer_5.z.toString()
        })

        xdk2mam.push({ sensor, data })
    }
    if (value.gyrometer_6) {
        sensor = 'Gyroscope'
        var data = []
        data.push({
            x: value.gyrometer_6.x.toString()
        })
        data.push({
            y: value.gyrometer_6.y.toString()
        })
        data.push({
            z: value.gyrometer_6.z.toString()
        })

        xdk2mam.push({ sensor, data })
    }

    return xdk2mam
}

module.exports = {
    convert2Xdk2mamFormat,
    publish
}