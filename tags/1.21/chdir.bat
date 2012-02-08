@ECHO OFF
IF NOT EXIST "compiled" (
	echo "creating output folder..."
	mkdir "compiled"
)
IF NOT EXIST "wtl" (
	echo "creating Windows Template Library directory"
	mkdir "wtl"
)
IF NOT EXIST "stlport" (
	echo "creating STLport directory"
	mkdir "stlport"
)

cd stlport

IF NOT EXIST "lib32" (
	mkdir "lib32"
)
if NOT EXIST "lib64" (
	mkdir "lib64"
)