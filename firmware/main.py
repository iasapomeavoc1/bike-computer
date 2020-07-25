# main python script to run on raspi 
# takes in data from serial communication with Arduino
# displays data on GUI 
# logs data to logging files
# runs GPS map display + navigation GUI
# logs camera data to video files
# displays mirrored rearview camera data on command?? 

import osmium as osm
import pandas as pd

class OSMHandler(osm.SimpleHandler):
    def __init__(self):
        osm.SimpleHandler.__init__(self)
        self.osm_data = []

    def tag_inventory(self, elem, elem_type):
        for tag in elem.tags:
            self.osm_data.append([elem_type, 
                                   elem.id, 
                                   elem.version,
                                   elem.visible,
                                   pd.Timestamp(elem.timestamp),
                                   elem.uid,
                                   elem.user,
                                   elem.changeset,
                                   len(elem.tags),
                                   tag.k, 
                                   tag.v])

    def node(self, n):
        self.tag_inventory(n, "node")

    def way(self, w):
        self.tag_inventory(w, "way")

    def relation(self, r):
        self.tag_inventory(r, "relation")


osmhandler = OSMHandler()
# scan the input file and fills the handler list accordingly
osmhandler.apply_file(' map.osm')

print(osmhandler.osm_data)

# transform the list into a pandas DataFrame
data_colnames = ['type', 'id', 'version', 'visible', 'ts', 'uid',
                 'user', 'chgset', 'ntags', 'tagkey', 'tagvalue']
df_osm = pd.DataFrame(osmhandler.osm_data, columns=data_colnames)
print(df_osm)
#df_osm = tag_genome.sort_values(by=['type', 'id', 'ts'])