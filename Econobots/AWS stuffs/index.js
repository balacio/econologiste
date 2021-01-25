const AWS = require('aws-sdk')

const firehose = new AWS.Firehose()
const StreamName = "SensorDataStream"

exports.handler = async (event) => {
    
    console.log('Received IoT event:', JSON.stringify(event, null, 2))
    
    let payload = {
        time: new Date(event.time),
        temperature: event.temperature,
        pressure: event.pressure,
        humidity: event.humidity
    }
    
    let params = {
            DeliveryStreamName: StreamName,
            Record: { 
                Data: JSON.stringify(payload)
            }
        }
        
    return await firehose.putRecord(params).promise()

}