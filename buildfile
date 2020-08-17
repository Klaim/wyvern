using build2@0.13.0

# Glue buildfile that "pulls" all the packages in the project.
#
import pkgs = */

./: $pkgs
