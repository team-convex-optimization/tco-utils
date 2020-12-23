function connection_modal_show(open_close) {
    const modal = document.getElementById("connectionModal");
    if (open_close == true) {
        modal.style.display = "block";
    }
    else {
        modal.style.display = "none";
    }
}
