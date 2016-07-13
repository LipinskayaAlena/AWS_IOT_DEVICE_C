#AWS_IOT_ROBOT_C

Robot moving project. It connects to AWS IoT platform using MQTT

#Prerequisities

Download certificate authority CA file from Symantec
Ensure you have created a thing through your AWS IoT Console with name matching the definition AWS_IOT_MY_THING_NAME in the aws_iot_config.h file
Ensure the names of the cert files are the same as in the aws_iot_config.h file
Ensure the certificate has an attached policy which allows the proper permissions for AWS IoT
#Connecting to the AWS IoT MQTT platform

  AWS_IoT_Client client;
  rc = aws_iot_mqtt_init(&client, &iotInitParams);
  rc = aws_iot_mqtt_connect(&client, &iotConnectParams);
Subscribe to a topic

  AWS_IoT_Client client;
  rc = aws_iot_mqtt_subscribe(&client, TOPICNAME, strlen(TOPICNAME), QOS0, iot_subscribe_callback_handler, NULL);
  
#Resources

API Documentation

#Config the field

Robot field config file:

Dimention
Field (1 - Wall)

5 6
1 1 1 1 1 
1 0 0 0 1
1 0 1 0 1
1 0 0 0 1
1 0 0 0 1
1 1 1 1 1
  
#Running

Build the project using Makefile(make)
Run the project
./robot -f CONFIG_FIELD_FILE
