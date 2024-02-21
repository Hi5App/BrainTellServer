rmdir /s /q "Generated"
mkdir "Generated"
mkdir "Generated/go"
mkdir "Generated/cpp"

"Bin/protobuf/bin/protoc.exe" -I=Bin/protobuf/include -I=proto proto/Message/* --plugin=protoc-gen-go=Bin/protobuf/bin/protoc-gen-go.exe --plugin=protoc-gen-go-grpc=Bin/protobuf/bin/protoc-gen-go-grpc.exe  --go_out=Generated/go --go-grpc_out=Generated/go

"Bin/protobuf/bin/protoc.exe" -I=Bin/protobuf/include -I=proto proto/Service/* --plugin=protoc-gen-go=Bin/protobuf/bin/protoc-gen-go.exe --plugin=protoc-gen-go-grpc=Bin/protobuf/bin/protoc-gen-go-grpc.exe  --go_out=Generated/go --go-grpc_out=Generated/go

"Bin/protobuf/bin/protoc.exe" -I=Bin/protobuf/include -I=proto proto/Service/* --plugin=protoc-gen-grpc-gateway=Bin/protobuf/bin/protoc-gen-grpc-gateway.exe --grpc-gateway_out Generated/go --grpc-gateway_opt paths=source_relative --grpc-gateway_opt generate_unbound_methods=true

rmdir /s /q "apiref"
mkdir "apiref"
mkdir "apiref/openapiv2"
"Bin/protobuf/bin/protoc.exe" -I=Bin/protobuf/include -I=proto proto/Service/* -I=Bin/protobuf/include/protoc-gen-openapiv2/options --plugin=protoc-gen-openapiv2=Bin/protobuf/bin/protoc-gen-openapiv2.exe --openapiv2_out apiref/openapiv2 --openapiv2_opt generate_unbound_methods=true

move "Generated\go\DBMS\SwcDbmsCommon\Generated\go\proto" "Generated\go\proto"
move "Generated\go\Service\Service.pb.gw.go" "Generated\go\proto\service"
rmdir /s /q "Generated\go\DBMS
rmdir /s /q "Generated\go\Service"

"Bin/protobuf/bin/protoc.exe" -I=Bin/protobuf/include -I=proto proto/Message/* --plugin=protoc-gen-grpc=Bin/protobuf/bin/grpc_cpp_plugin.exe --cpp_out=Generated/cpp --grpc_out=Generated/cpp

"Bin/protobuf/bin/protoc.exe" -I=Bin/protobuf/include -I=proto proto/Service/* --plugin=protoc-gen-grpc=Bin/protobuf/bin/grpc_cpp_plugin.exe --cpp_out=Generated/cpp --grpc_out=Generated/cpp

