source ./common.sh

rm -rf $OUT
rm -rf $PROJECT_DIR
mkdir -p $OUT

advixe-cl -create-project --project-dir $PROJECT_DIR test
