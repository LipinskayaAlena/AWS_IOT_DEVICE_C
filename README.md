#AWS_IOT_DEVICE_C

Robot moving project. It connects to AWS IoT platform using MQTT



#Prerequisities
Create a new AWS IoT Thing https://www.youtube.com/watch?v=hOc-iZcmv9E </br>
Download AWS IoT SDK Embedded C https://github.com/aws/aws-iot-device-sdk-embedded-C </br>
Download certificate authority CA file from Symantec </br>
Ensure you have created a thing through your AWS IoT Console with name matching the definition AWS_IOT_MY_THING_NAME in the aws_iot_config.h file </br>
Ensure the names of the cert files are the same as in the aws_iot_config.h file </br>
Ensure the certificate has an attached policy which allows the proper permissions for AWS IoT </br>
#Create a thing through AWS IoT Console
1. AWS IoT Console - https://console.aws.amazon.com/iot/home </br>
2. Create a Device, Create and Activate a Device Certificate, Create an AWS IoT Policy and attach it to a device https://www.youtube.com/watch?v=hOc-iZcmv9E </br>

#Connecting to the AWS IoT MQTT platform
```
  AWS_IoT_Client client;
  rc = aws_iot_mqtt_init(&client, &iotInitParams);
  rc = aws_iot_mqtt_connect(&client, &iotConnectParams); 
```
Subscribe to a topic </br>
``
  AWS_IoT_Client client; 
  rc = aws_iot_mqtt_subscribe(&client, TOPICNAME, strlen(TOPICNAME), QOS0, iot_subscribe_callback_handler, NULL); 
`` </br>
Update Thing Shadow from a device </br>
`
rc = aws_iot_shadow_update(&mqttClient, AWS_IOT_MY_THING_NAME, pJsonDocumentBuffer, ShadowUpdateStatusCallback,
                            pCallbackContext, TIMEOUT_4SEC, persistenSubscription);
`
#Resources

API Documentation
http://aws-iot-device-sdk-java-docs.s3-website-us-east-1.amazonaws.com/

#Config the field

Robot field config file:

Dimention
Field (1 - Wall)
````
5 6
1 1 1 1 1 
1 0 0 0 1
1 0 1 0 1
1 0 0 0 1
1 0 0 0 1
1 1 1 1 1
````  
#Running

Build the project using Makefile(make) </br>
``
Run the project
./robot
``
