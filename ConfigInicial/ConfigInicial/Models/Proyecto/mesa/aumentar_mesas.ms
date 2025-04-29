--------------------------------------------------------------------------------
-- 1) Localizar la mesa base
--------------------------------------------------------------------------------
baseMesa = getNodeByName "mesa"
if baseMesa == undefined then (
    messageBox "No encontré ningún objeto llamado \"mesa\" en la escena."
) else (
    --------------------------------------------------------------------------------
    -- 2) Clonar 20 veces y renombrar mesa1, mesa2, …, mesa20
    --------------------------------------------------------------------------------
    clones = for i = 1 to 20 collect (
        -- copia la malla, puedes usar `copy` o `instance`
        newName = "mesa" + i as string
        inst = copy baseMesa name:newName
        inst
    )
    print ("Clonadas " + clones.count as string + " mesas.")
)
