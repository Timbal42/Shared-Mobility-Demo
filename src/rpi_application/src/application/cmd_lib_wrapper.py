import ctypes


    
def setup_se_interface():
    _lib = ctypes.CDLL("../../build/src/se_interface/libse_interface.so")
    
    _lib.wrap_block2go_select.argtypes = [ ctypes.POINTER(ctypes.c_uint8),
                                           ctypes.POINTER(ctypes.c_char_p)]
    _lib.wrap_gen_key.argtypes = [  ctypes.POINTER(ctypes.c_uint8) ]
    
    _lib.wrap_get_pub_key.argtypes = [ ctypes.c_uint8, 
                                       ctypes.POINTER(ctypes.POINTER(ctypes.c_uint8)),
                                       ctypes.POINTER(ctypes.c_uint8)] 
                                       
    _lib.wrap_sign.argtypes = [ ctypes.c_uint8, 
                                ctypes.POINTER(ctypes.c_uint8),
                                ctypes.POINTER(ctypes.POINTER(ctypes.c_uint8)),
                                ctypes.POINTER(ctypes.c_uint16) ]
    
    return _lib

def init():
    global se_interface
    se_interface = setup_se_interface()
    ret = se_interface.se_interface_init()
    if(ret):
        print(ret)
        print("interface init error\n")
    
    return ret
    

def block2go_select():
    
    id_nr = (ctypes.c_uint8 * 11)()
    version = (ctypes.c_char_p)()
    
    ret = se_interface.wrap_block2go_select(id_nr, version)
    if(ret):
        print("select error\n")
    
    return ret;

def block2go_gen_key():
    c_key_index = (ctypes.c_uint8 * 1)()
    ret = se_interface.wrap_gen_key(c_key_index)
    if(ret):
        print("genkeys error\n")
    
    return c_key_index[0]

def block2go_get_pub_key(key_index):
    c_key_index = ctypes.c_uint8()
    c_key_index.value = key_index
    c_public_key = ctypes.POINTER(ctypes.c_uint8)()
    c_public_key_len = ctypes.c_uint8()
   
    ret = se_interface.wrap_get_pub_key(c_key_index, c_public_key, c_public_key_len)

    if(ret):
        print("get public key error\n")
        
    return c_public_key[0:c_public_key_len.value]
    
def block2go_sign(key_index, msg):
    print("[SE] sign");
    c_key_index = ctypes.c_uint8()
    c_key_index.value = key_index
    c_msg = (ctypes.c_uint8) * 32
    c_msg = c_msg(*msg)
    c_sig = ctypes.POINTER(ctypes.c_uint8)()
    c_sig_len = ctypes.c_uint16()
    
    ret = se_interface.wrap_sign(key_index, c_msg, c_sig, c_sig_len)
    
    if(ret):
        print("sign error\n")
    
    return c_sig[0:c_sig_len.value]
    
