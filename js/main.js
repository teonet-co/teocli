console.info("node.js adapter to teo");

teoclient = require("/home/aksenofo/PROJECT/TEOCLI/teocli/js/node_modules/teoclient/build/Debug/teoclient.node")

//m = teoclient

//var p = new m.Person("Taro");
//console.log(p.name);
//p.name = "Jiro111";
//console.log("dddddd", p.getName());

//console.log(teoclient.version())

teoclient.init();

try {

    var connector = teoclient.connect("127.0.0.1", 9000)
    console.log(connector);
    connector.login();

//    var c = new teoclient.Connector("127.0.0.1", 9000);
//    c.Login();

//    console.log(JSON.stringify(connector));
//    connector.Login();

//    console.log(teoclient.connect("ederere", "werererer"))
/*
console.log(teoclient.version())
teoclient.init();
teoclient.cleanup();

teoclient.Connector()

//teoclient.connect()

a = new teoclient.TeoExeption(100, "p1", "p2")

a.errno = 10000
a.call_name = "HHHHH"
a.text = "TTTTTT"
console.log(a.errno)
console.log(a.call_name)
console.log(a.text)
console.log(a)

console.log(teoclient.connect("ederere", "werererer"))

*/

}
catch (err) {
    console.log(err)
}

teoclient.cleanup();
