/* MAXScript: clonar y transformar solo las mesas (desk1…desk20) */

/* 1) Clonar la mesa base 20 veces */
baseMesa = getNodeByName "mesa"
if baseMesa == undefined then (
    messageBox "No encontré un objeto llamado \"mesa\"."
) else (
    for i = 1 to 20 do (
        copy baseMesa name:("desk" + i as string)
    )
    print "Clonadas 20 mesas (desk1…desk20)."
)

/* 2) Tabla de transformaciones solo para las mesas */
/*    Cada elemento: #("nombreNodo", [posX,posY,posZ], rotY, [sclX,sclY,sclZ]) */
transformData = #(
    #("desk1",  [-25.0, 5.7, -23.0],  90.0, [6.0,6.0,6.0]),
    #("desk2",  [-25.0, 5.7, -17.0],  90.0, [6.0,6.0,6.0]),
    #("desk3",  [-25.0, 5.7,  13.0],  90.0, [6.0,6.0,6.0]),
    #("desk4",  [-25.0, 5.7,  23.0],  90.0, [6.0,6.0,6.0]),
    #("desk5",  [ -6.0, 5.7,  23.0],  90.0, [6.0,6.0,6.0]),
    #("desk6",  [ -6.0, 5.7,  13.0],  90.0, [6.0,6.0,6.0]),
    #("desk7",  [ -6.0, 5.7, -13.0],  90.0, [6.0,6.0,6.0]),
    #("desk8",  [ -6.0, 5.7, -23.0],  90.0, [6.0,6.0,6.0]),
    #("desk9",  [ 13.0, 5.7, -13.0],  90.0, [6.0,6.0,6.0]),
    #("desk10", [ 13.0, 5.7, -23.0],  90.0, [6.0,6.0,6.0]),
    #("desk11", [ 13.0, 5.7,  13.0],  90.0, [6.0,6.0,6.0]),
    #("desk12", [ 13.0, 5.7,  23.0],  90.0, [6.0,6.0,6.0]),
    #("desk13", [ 32.0, 5.7,  23.0],  90.0, [6.0,6.0,6.0]),
    #("desk14", [ 32.0, 5.7,  13.0],  90.0, [6.0,6.0,6.0]),
    #("desk15", [ 32.0, 5.7, -13.0],  90.0, [6.0,6.0,6.0]),
    #("desk16", [ 32.0, 5.7, -23.0],  90.0, [6.0,6.0,6.0]),
    #("desk17", [ 51.0, 5.7, -23.0],  90.0, [6.0,6.0,6.0]),
    #("desk18", [ 51.0, 5.7, -13.0],  90.0, [6.0,6.0,6.0]),
    #("desk19", [ 51.0, 5.7,  13.0],  90.0, [6.0,6.0,6.0]),
    #("desk20", [ 51.0, 5.7,  23.0],  90.0, [6.0,6.0,6.0])
)

/* 3) Función para aplicar la transformación a cada mesa */
fn applyTransform nodeName pos rotY scl = (
    local obj = getNodeByName nodeName
    if obj != undefined then (
        obj.position = pos
        obj.rotation = eulerAngles 0 rotY 0
        obj.scale    = scl
    ) else (
        format "? No encontré el nodo: %\n" nodeName
    )
)

/* 4) Recorrer transformData y aplicar a todos los desks */
for t in transformData do (
    applyTransform t[1] t[2] t[3] t[4]
)

print "?? Transformaciones aplicadas solo a las mesas (desk1…desk20)."
