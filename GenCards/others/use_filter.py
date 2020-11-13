#See : https://cp3.irmp.ucl.ac.be/projects/madgraph/wiki/FAQ-General-15
#Remove diagrams not containing a top quark
#Add '--diagram_filter' option to 'generate' command in proc_card.dat

import madgraph.various.misc as misc

def remove_diag(diag):
    """force to have a top quark in the diagram
    """

    found_top = False
    for vertex in diag['vertices']:
        if vertex.get('id') == 0: #special final vertex
            continue

        for idx, leg in enumerate(list(vertex['legs'])):
            if abs(leg['id']) == 6:
                found_top = True
                break

    return not found_top
