require(data.table)
require(rgdal)

# Read the CNEP+ data set and sample PWID, and then create a lat/lon for each
#  based on their zipcode,



CENP_100k <- fread("100k_CNEP+_pwid_catalog_2018-11-23.csv")
num_pwid <- 32000


dtp <- CENP_100k[sample(.N,num_pwid)]


nperzip = dtp[,.N,by = Zip][order(-N)]
# create index to match zips.shp
# need all of Illinois or area
# rbind SpatialPolygonsDataFrame: https://stackoverflow.com/questions/31108927/combining-spatialpolygonsdataframe-of-two-neighbour-countries
# adm1.spdf <- rbind(Pakistan.adm1.spdf, India.adm1.spdf, makeUniqueIDs = TRUE)
# plot(adm1.spdf)

# loading all us ZIP codes

#allzips.shp <- readOGR("../../gisdata/tl_2017_us_zcta510/tl_2017_us_zcta510.shp")
#allzipsidx = allzips.shp@data$ZCTA5CE10

allzips.shp <- readOGR("../../gisdata/illinois_zips/zt17_d00.shp")
allzipsidx = allzips.shp@data$ZCTA

# azi are the all zip indices for the ZIP code in nperzip
nperzip[,azi := match(Zip,allzipsidx)]

# all points
allpoints = nperzip[,data.table(coordinates(spsample(allzips.shp[azi,], n = N, type = "random"))),by = Zip]

# This works, see alternative slower way below.
all.agent.points1 = cbind(dtp[order(match(Zip,allpoints$Zip))],allpoints)
setnames(all.agent.points1, c("x","y"), c("lon","lat"))
fwrite(all.agent.points1,file = "pwids_with_lat_lon_original_zips.csv")
