const express=require('express');
const app=express();
const cores=require('cors');
app.use(cores());
app.use(express.json());

let devices={};
app.post("/devices",(req,res)=>{
const data=req.body;
devices[data.device_id]={
    ...data,
    lastseen:new Date()
};
console.log("Received data from ESP32: ",data);
res.sendStatus(200).json({status:"OK"});
}
);
app.get("/devices",(req,res)=>{
    res.json(devices);
});
app.get("/",(req,res)=>{
    res.send("Cloud server is running");
});
const port=process.env.port||3000;
app.listen(port,()=>{
console.log(`Cloud server is running on port ${port}`);
});
