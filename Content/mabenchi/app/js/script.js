const overlay= document.getElementById("overlay");
const menu= document.getElementById("menu");
const body= document.querySelector("body");
document.getElementById("humburger").addEventListener("click", function (){
    if(humburger.classList.contains("open")){
        body.classList.remove("no-scroll")
        menu.classList.remove("visible-menu");
        overlay.classList.remove("visible-overlay")
        humburger.classList.remove("open");

    }

   
    else{
        body.classList.add("no-scroll")
        menu.classList.add("visible-menu");

        overlay.classList.add("visible-overlay")        
        window.scrollTo(0, 0)

        humburger.classList.add("open");

    }
    
})

