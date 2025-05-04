/* MAXScript: colocar todas las mesas (desk1…desk20) en el plano XY (Z = 0) */

for i = 1 to 20 do (
    local nodeName = "desk" + i as string
    local obj = getNodeByName nodeName

    if obj != undefined then (
        -- Mantener X,Y actuales, forzar Z = 0
        obj.position = [ obj.position.x, obj.position.y, 0 ]
    ) else (
        format "? No encontré el nodo: %\n" nodeName
    )
)

print "?? Todas las mesas han sido movidas al plano XY (Z=0)."
