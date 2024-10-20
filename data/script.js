function init() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function()
    {
        if(this.readyState == 4 && this.status == 200)
        {
            const etatInit = this.responseText.split(";");
            if(etatInit[0] == "1") document.getElementById("E1").checked = true;
            document.getElementById("EV1").value = etatInit[1];
            valeurEV1(etatInit[1]);
        }
    };
    xhttp.open("GET", "init", true);
    xhttp.send();
}

function majE1() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "e1", true);
    xhttp.send();
}

function valeurEV1(valeur) {
    document.getElementById("valeurEV1").innerHTML = valeur;
}

function valeurEV2(valeur) {
    document.getElementById("valeurEV2").innerHTML = valeur;
}

function majEV1(valeur) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "ev1?v=" + valeur, true);
    xhttp.send();
}

function majEV2(valeur) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "ev2?v=" + valeur, true);
    xhttp.send();
}

function majV(valeur) {
    majV1(valeur);
    setTimeout(majV2(valeur), 50);
    setTimeout(majV3(valeur), 50);
}

function majV1(valeur) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "v1?" + valeur, true);
    xhttp.send();
}

function majV2(valeur) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "v2?" + valeur, true);
    xhttp.send();
}

function majV3(valeur) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "v3?" + valeur, true);
    xhttp.send();
}