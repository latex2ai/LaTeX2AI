$(function () {
    var csInterface = new CSInterface()

    // Set the current skin colors
    updateThemeWithAppSkinInfo(csInterface.hostEnvironment.appSkinInfo)

    // Update the color of the panel when the theme color of the product changed
    csInterface.addEventListener(
        CSInterface.THEME_COLOR_CHANGED_EVENT,
        onAppThemeColorChanged
    )

    // Add all the event listeners we need
    csInterface.addEventListener(
        "com.adobe.csxs.events.latex2ai.debug.update",
        update_create_form
    )
    csInterface.addEventListener(
        "com.adobe.csxs.events.latex2ai.debug.close",
        csInterface.closeExtension
    )

    // Set the functions for the possible actions on the form
    $("#button_ok").click(function (event) {
        event.preventDefault()
        var event = new CSEvent(
            "com.adobe.csxs.events.latex2ai.debug.ok",
            "APPLICATION",
            "ILST",
            "LaTeX2AIUI"
        )
        event.data = ""
        csInterface.dispatchEvent(event)
    })

    $("#button_open_log").click(function (event) {
        event.preventDefault()
        var event = new CSEvent(
            "com.adobe.csxs.events.latex2ai.debug.open_log",
            "APPLICATION",
            "ILST",
            "LaTeX2AIUI"
        )
        event.data = ""
        csInterface.dispatchEvent(event)
    })

    $("#button_create_debug").click(function (event) {
        event.preventDefault()
        var event = new CSEvent(
            "com.adobe.csxs.events.latex2ai.debug.create_debug",
            "APPLICATION",
            "ILST",
            "LaTeX2AIUI"
        )
        event.data = ""
        csInterface.dispatchEvent(event)
    })

    $("#button_cancel").click(function (event) {
        event.preventDefault()
        var event = new CSEvent(
            "com.adobe.csxs.events.latex2ai.debug.cancel",
            "APPLICATION",
            "ILST",
            "LaTeX2AIUI"
        )
        event.data = ""
        csInterface.dispatchEvent(event)
    })
    document.addEventListener("keydown", (event) => {
        // Somehow when "Esc" is pressed we get an empty string as return value rom event.key (instead of the expected "Escape").
        // The variable event.code is also empty, but it seems like this one is only empty for escape (space also produces an
        // empty event.key but NOT an empty event.code).
        if (event.code === "") {
            event.preventDefault()
            var event = new CSEvent(
                "com.adobe.csxs.events.latex2ai.debug.ok",
                "APPLICATION",
                "ILST",
                "LaTeX2AIUI"
            )
            event.data = ""
            csInterface.dispatchEvent(event)
        }
    })

    // let the native plug-in part of this sample know that we are ready to receive events now.
    var panelReadyEvent = new CSEvent(
        "com.adobe.csxs.events.latex2ai.debug.ready",
        "APPLICATION",
        "ILST",
        "LaTeX2AIUI"
    )
    csInterface.dispatchEvent(panelReadyEvent)
})

function update_create_form(event) {
    var xmlData = $.parseXML(event.data)
    var $xml = $(xmlData)

    check_git_hash($xml)

    var l2a_xml = $xml.find("form_data")

    action = l2a_xml.attr("action")
    if (action == "redo_items") {
        $("#button_ok").prop("disabled", true)
        $("#extra_text").prop(
            "innerHTML",
            "The error occurred while recompiling items that were not changed.\nThis usually happens when something in the header changes or the document is compiled on a different system than before."
        )
    } else if (action == "item_create") {
        $("#button_cancel").val("Cancel item creation")
    } else if (action == "item_edit") {
        $("#button_cancel").val("Cancel item edit")
    }
}
