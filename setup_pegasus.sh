export PEGASUS_ROOT=`pwd`
export PEGASUS_HOME=`pwd`/pkg
export PEGASUS_PLATFORM=LINUX_X86_64_CLANG
export PEGASUS_DEBUG=yes
export PEGASUS_ENABLE_CQL=true
export PEGASUS_DEFAULT_ENABLE_OOP=true
export PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER=true
export PEGASUS_ENABLE_IPV6=true
export PEGASUS_ENABLE_INTEROP_PROVIDER=true
export PEGASUS_HAS_SSL=yes
export PEGASUS_ENABLE_SLP=true
export PEGASUS_USE_EXTERNAl_SLP=true
export PEGASUS_ENABLE_PROTOCOL_WSMAN=true
export PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT=true
export PEGASUS_ENABLE_EMAIL_HANDLER=true
export PATH=$PATH:$PEGASUS_HOME/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PEGASUS_HOME/lib

#Required for the generation of spec files generation only
#export OPENSSL_HOME=/usr
#export OPENSSL_BIN=/usr/bin
#export PEGASUS_ENVVAR_FILE=/home/deleisha/Codespace/pegasus/env_var_Linux.status
