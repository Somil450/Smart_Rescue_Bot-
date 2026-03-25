import 'package:flutter/material.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';
import 'dart:convert';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: ControlPage(),
      debugShowCheckedModeBanner: false,
    );
  }
}

class ControlPage extends StatefulWidget {
  @override
  _ControlPageState createState() => _ControlPageState();
}

class _ControlPageState extends State<ControlPage> {

  BluetoothConnection? connection;

  String temperature = "--";
  String distance = "--";
  String fire = "SAFE";

  void connect() async {

    List<BluetoothDevice> devices =
        await FlutterBluetoothSerial.instance.getBondedDevices();

    BluetoothDevice device =
        devices.firstWhere((d) => d.name == "SmartBot");

    connection = await BluetoothConnection.toAddress(device.address);

    connection!.input!.listen((data) {

      String msg = String.fromCharCodes(data);

      if(msg.contains("TEMP"))
        temperature = msg.split(":")[1];

      if(msg.contains("DIST"))
        distance = msg.split(":")[1];

      if(msg.contains("FLAME"))
        fire = msg.split(":")[1];

      setState(() {});

    });

  }

  void sendCommand(String cmd) {

    connection!.output.add(utf8.encode(cmd + "\n"));

  }

  @override
  Widget build(BuildContext context) {

    return Scaffold(
      appBar: AppBar(
        title: Text("Smart Rescue Bot"),
      ),

      body: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [

          Text("Temperature : $temperature °C", style: TextStyle(fontSize:20)),
          Text("Distance : $distance cm", style: TextStyle(fontSize:20)),
          Text("Fire : $fire", style: TextStyle(fontSize:20)),

          SizedBox(height:30),

          ElevatedButton(
            onPressed: connect,
            child: Text("Connect to ESP32"),
          ),

          SizedBox(height:40),

          IconButton(
            icon: Icon(Icons.arrow_upward, size:50),
            onPressed: (){
              sendCommand("forward");
            },
          ),

          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [

              IconButton(
                icon: Icon(Icons.arrow_back, size:50),
                onPressed: (){
                  sendCommand("left");
                },
              ),

              IconButton(
                icon: Icon(Icons.stop, size:50),
                onPressed: (){
                  sendCommand("stop");
                },
              ),

              IconButton(
                icon: Icon(Icons.arrow_forward, size:50),
                onPressed: (){
                  sendCommand("right");
                },
              ),

            ],
          ),

          IconButton(
            icon: Icon(Icons.arrow_downward, size:50),
            onPressed: (){
              sendCommand("back");
            },
          ),

        ],
      ),
    );
  }
}