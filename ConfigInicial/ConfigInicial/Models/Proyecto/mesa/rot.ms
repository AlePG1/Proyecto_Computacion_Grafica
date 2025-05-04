/* MAXScript: rotar todas las mesas (desk1…desk20) para que queden de pie (patas verticales) */

/* Recorre cada mesa y aplica un giro de 90° alrededor del eje X */
for i = 1 to 20 do (
    local nodeName = "desk" + i as string
    local obj = getNodeByName nodeName

    if obj != undefined then (
        -- Gira 90° alrededor de X para poner las patas en vertical
        rotate obj (angleaxis 90 [1,0,0])
    ) else (
        format "? No encontré el nodo: %\n" nodeName
    )
)

print "?? Todas las mesas han sido rotadas para quedar de pie." 
