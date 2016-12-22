
class TypedRead:
    def __init__( self, ty, nm, buf_size_in_cl = "128", max_burst_count = "1<<24", buf_size_in_burst_reqs = None):
        self.ty = ty
        self.nm = nm
        self.buf_size_in_cl = buf_size_in_cl
        self.max_burst_count = max_burst_count
        if buf_size_in_burst_reqs is None:
            buf_size_in_burst_reqs = buf_size_in_cl
        self.buf_size_in_burst_reqs = buf_size_in_burst_reqs

    def loadUnitType( self):
        return "LoadUnitParams< %s, %s, %s, %s>" % (self.ty,self.buf_size_in_cl,self.max_burst_count,self.buf_size_in_burst_reqs)

    def reqTy( self):
        return "MemTypedReadReqType<%s>" % self.ty

    def respTy( self):
        return "MemTypedReadRespType<%s>" % self.ty

    def reqNm( self):
        return "%sReq" % self.nm

    def respNm( self):
        return "%sResp" % self.nm

    def reqNmT( self):
        return self.reqNm() + "In"

    def respNmT( self):
        return self.respNm() + "Out"

    def reqNmK( self):
        return self.reqNm() + "Out"

    def respNmK( self):
        return self.respNm() + "In"

class SingleRead:
    def __init__( self, ty, tag_ty, nm, buf_size = "128"):
        self.ty = ty
        self.tag_ty = tag_ty
        self.nm = nm
        self.buf_size = buf_size

    def loadUnitType( self):
        return "LoadUnitSingleReqParams< %s, %s, %s>" % (self.ty,self.tag_ty,self.buf_size)

    def reqTy( self):
        return "MemSingleReadReqType<%s,%s>" % (self.ty, self.tag_ty)

    def respTy( self):
        return "MemSingleReadRespType<%s,%s>" % (self.ty, self.tag_ty)

    def reqNm( self):
        return "%sReq" % self.nm

    def respNm( self):
        return "%sResp" % self.nm

    def reqNmT( self):
        return self.reqNm() + "In"

    def respNmT( self):
        return self.respNm() + "Out"

    def reqNmK( self):
        return self.reqNm() + "Out"

    def respNmK( self):
        return self.respNm() + "In"

class TypedWrite:
    def __init__( self, ty, nm):
        self.ty = ty
        self.nm = nm

    def storeUnitType( self):
        return "StoreUnitParams< %s>" % (self.ty,)

    def reqTy( self):
        return "MemTypedWriteReqType<%s>" % self.ty

    def dataTy( self):
        return "MemTypedWriteDataType<%s>" % self.ty

    def reqNm( self):
        return "%sReq" % self.nm

    def dataNm( self):
        return "%sData" % self.nm

    def reqNmT( self):
        return self.reqNm() + "In"

    def dataNmT( self):
        return self.dataNm() + "In"

    def reqNmK( self):
        return self.reqNm() + "Out"

    def dataNmK( self):
        return self.dataNm() + "Out"

class ArrayField:
    def __str__( self): return "ArrayField<" + str(self.ty) + "," + str(self.count) + ">"

    def __init__( self, ty, count):
        self.ty = ty
        self.count = count

    @property
    def declaration( self):
        return "%s %s[%d];" % (self.ty.ty, self.nm, self.count)

    def fieldWidth( self, sum):
        if type(self.ty) is UserTypeField:
          ln = self.ty.user_ty.numberOfFields
          return "  return %s::fieldWidth( (index-%d) %% %d);" % (self.ty.user_ty.ty, sum, ln)
        else:
          return "  return %d;" % (self.ty.bitwidth,)

    def putField( self, sum):
        if type(self.ty) is UserTypeField:
          ln = self.ty.user_ty.numberOfFields
          return "  %s[(index-%d)/%d].putField( (index-%d) %% %d, d);" % (self.nm, sum, ln, sum, ln)
        else:
          return "  %s[index-%d] = d;" % (self.ty.nm,sum)

    def getField( self, sum):
        if type(self.ty) is UserTypeField:
          ln = self.ty.user_ty.numberOfFields
          return "  return %s[(index-%d)/%d].getField( (index-%d) %% %d);" % (self.nm, sum, ln, sum, ln)
        else:
          return "  return %s[index-%d];" % (self.ty.nm,sum)

    @property
    def nm( self):
        return self.ty.nm

    @property
    def numberOfFields( self):
        return self.count * self.ty.numberOfFields
    
    @property
    def bitwidth( self):
        return self.count * self.ty.bitwidth

class BasicField:
    def __init__( self, ty, nm, tag, bitwidth):
        self.ty = ty
        self.nm = nm
        self.tag = tag
        self.bitwidth = bitwidth

    def __str__( self):
        return self.tag

    @property
    def declaration( self):
        return "%s %s;" % (self.ty, self.nm)

    def fieldWidth( self, sum):
        return "  return %d;" % self.bitwidth

    def putField( self, sum):
        return "  %s = d;" % self.nm

    def getField( self, sum):
        return "  return %s;" % self.nm

    @property
    def numberOfFields( self):
        return 1

class BitReducedField(BasicField):
    def __str__( self): return "BitReducedField<" + str(self.ty) + "," + str(self.bitwidth) + ">"

    def __init__( self, ty, bitwidth):
        assert ty.bitwidth >= bitwidth
        super().__init__( ty, ty.nm, "", bitwidth)

    @property
    def declaration( self):
        return "%s %s : %d;" % (self.ty.ty, self.nm, self.bitwidth)

class UnsignedLongLongField(BasicField):
    def __init__( self, nm):
        super().__init__( "unsigned long long", nm, "UnsignedLongLongField", 64)

class SignedLongLongField(BasicField):
    def __init__( self, nm):
        super().__init__( "long long", nm, "SignedLongLongField", 64)

class UnsignedIntField(BasicField):
    def __init__( self, nm):
        super().__init__( "unsigned int", nm, "UnsignedIntField", 32)

class SignedIntField(BasicField):
    def __init__( self, nm):
        super().__init__( "int", nm, "SignedIntField", 32)

class UnsignedShortField(BasicField):
    def __init__( self, nm):
        super().__init__( "unsigned short", nm, "UnsignedShortField", 16)

class SignedShortField(BasicField):
    def __init__( self, nm):
        super().__init__( "short", nm, "SignedShortField", 16)

class UnsignedCharField(BasicField):
    def __init__( self, nm):
        super().__init__( "unsigned char", nm, "UnsignedCharField", 8)

class SignedCharField(BasicField):
    def __init__( self, nm):
        super().__init__( "char", nm, "SignedCharField", 8)

class UserTypeField:
    def __init__( self, nm, user_ty):
        self.nm = nm
        self.user_ty = user_ty

    @property
    def declaration( self):
        return "%s %s;" % (self.ty, self.nm)

    def fieldWidth( self, sum):
        return "  return %s::fieldWidth( index-%d);" % (self.ty, sum)

    def putField( self, sum):
        return "  %s.putField( index-%d, d);" % (self.nm, sum)

    def getField( self, sum):
        return "  return %s.getField( index-%d);" % (self.nm, sum)

    @property
    def ty( self):
        return self.user_ty.ty

    @property
    def numberOfFields( self):
        return self.user_ty.numberOfFields

    @property
    def bitwidth( self):
        return self.user_ty.bitwidth


class UserType:
    def __init__( self, ty, fields):
        self.ty = ty
        self.fields = fields

        sum = 0
        for field in fields:
            field.offset = sum
            sum += field.numberOfFields

    @property
    def numberOfFields( self):
        sum = 0
        for field in self.fields:
            sum += field.numberOfFields
        return sum

    @property
    def bitwidth( self):
        sum = 0
        for field in self.fields:
            sum += field.bitwidth
        return sum


from collections import OrderedDict

class StorageFIFO:
    def __init__( self, ty, capacity, nm):
        self.ty = ty
        self.capacity = capacity
        self.nm = nm

class Port:
    def __init__( self, channel):
        self.channel = channel
    def __eq__( self, other):
        return type(self) == type(other) and self.channel == other.channel

class RdReqPort(Port):
    def __init__( self, channel):
        super().__init__( channel)
    @property
    def reset( self):
        return self.channel + "ReqOut.reset_put()"
    def __repr__( self):
        return "RdReqPort(" + self.channel + ")"

class RdRespPort(Port):
    def __init__( self, channel):
        super().__init__( channel)
    @property
    def reset( self):
        return self.channel + "RespIn.reset_get()"
    def __repr__( self):
        return "RdRespPort(" + self.channel + ")"


class WrReqPort(Port):
    def __init__( self, channel):
        super().__init__( channel)
    @property
    def reset( self):
        return self.channel + "ReqOut.reset_put()"
    def __repr__( self):
        return "WrReqPort(" + self.channel + ")"

class WrDataPort(Port):
    def __init__( self, channel):
        super().__init__( channel)
    @property
    def reset( self):
        return self.channel + "DataOut.reset_put()"
    def __repr__( self):
        return "WrDataPort(" + self.channel + ")"

class EnqueuePort(Port):
    def __init__( self, channel):
        super().__init__( channel)
    @property
    def reset( self):
        return self.channel + ".reset_put()"
    def __repr__( self):
        return "EnqueuePort(" + self.channel + ")"

class DequeuePort(Port):
    def __init__( self, channel):
        super().__init__( channel)
    @property
    def reset( self):
        return self.channel + ".reset_get()"
    def __repr__( self):
        return "DequeuePort(" + self.channel + ")"

class CThread:
    def __init__( self, nm, ports=None, writes_to_done=False):
        self.nm = nm
        if ports is None:
            self.ports = []
        else:
            self.ports = ports
        self.writes_to_done = writes_to_done

    def add_port( self, p):
        self.ports.append( p)
        return self

    def add_ports( self, ps):
        self.ports.extend( ps)
        return self

class Module:
    def __init__( self, nm):
        self.nm = nm
        self.cthreads = OrderedDict()
        self.storage_fifos = []
        self.modules = OrderedDict()

    @property
    def writes_to_done( self):
        for c in self.cthreads.values():
            if c.writes_to_done:
                return True
        return False

    def add_module( self, v): self.modules[v.nm] = v
    def add_modules( self, vs):
        for v in vs:
            self.add_modules( v)
    def get_module( self, nm): return self.modules[nm]

    def add_cthread( self, v): self.cthreads[v.nm] = v
    def add_cthreads( self, vs):
        for v in vs:
            self.add_cthread( v)
    def get_cthread( self, nm): return self.cthreads[nm]

    def add_storage_fifo( self, v): self.storage_fifos.append( v)        
    def add_storage_fifos( self, vs): self.storage_fifos.extend( vs)        

    def portOf( self, p_in):
        for c in self.cthreads.values():
            for p in c.ports:
                if p == p_in:
                    return True
        return False

class DUT:
    def __init__( self, nm):
        self.module = Module( nm)
        self.inps = []
        self.outs = []
        self.usertypes = OrderedDict()
        self.extra_config_fields = []

    @property
    def nm( self):
        return self.module.nm

    @property
    def modules( self):
        return self.module.modules

    @property
    def cthreads( self):
        return self.module.cthreads

    @property
    def cthreads_generator( self):
        for c in self.module.cthreads.values():
            yield c
        for m in self.module.modules.values():
            for c in m.cthreads.values():
                yield c

    @property
    def storage_fifos( self):
        for f in self.module.storage_fifos:
          th0 = self.put_tbl[f.nm]
          th1 = self.get_tbl[f.nm]
          p0 = self.find_parent(th0)
          p1 = self.find_parent(th1)
          if p0.nm == p1.nm:
             yield f

    @property
    def tlm_fifos( self):
        for f in self.module.storage_fifos:
          th0 = self.put_tbl[f.nm]
          th1 = self.get_tbl[f.nm]
          p0 = self.find_parent(th0)
          p1 = self.find_parent(th1)
          if p0.nm != p1.nm:
            yield f

    def isHier( self, p_in):
        if self.module.portOf( p_in):
            return ""
        return "hier_"

    def add_module( self, v):
        self.module.add_module( v)
        return self

    def add_modules( self, vs):
        for v in vs:
            self.add_module( v)
        return self

    def get_module( self, nm): return self.module.get_module( nm)

    def add_cthread( self, v): self.module.add_cthread( v)
    def get_cthread( self, nm):
        if nm in self.module.cthreads:
            return self.module.get_cthread( nm)
        else:
            for m in self.module.modules.values():
                if nm in m.cthreads:
                    return m.get_cthread( nm)
            assert None

    def find_parent( self, nm):
        if nm in self.module.cthreads:
            return self.module
        else:
            for m in self.module.modules.values():
                if nm in m.cthreads:
                    return m
            assert None

    def add_storage_fifo( self, v): self.module.add_storage_fifo( v)
    def add_storage_fifos( self, vs):
        for v in vs:
            self.add_storage_fifo( v)

    def semantic(self):
        config_bitwidth = 64*(len(self.inps)+len(self.outs))
        for field in self.extra_config_fields:
            config_bitwidth += field.bitwidth
#        print( "// Config contains %d bits" % config_bitwidth)
#        print( "// Config contains %d bytes" % (config_bitwidth/8))
        assert( config_bitwidth <= 9*64)

        # Find two ports for each storage fifo
        # And req,resp for each rd
        # And req,data for each wr
        self.rd_req_tbl = {}
        self.rd_resp_tbl = {}
        self.wr_req_tbl = {}
        self.wr_data_tbl = {}

        self.get_tbl = {}
        self.put_tbl = {}

#
# Current only two level
#
        all_cthreads = list(self.module.cthreads.values())
        for m in self.modules.values():
            all_cthreads.extend( m.cthreads.values())

        for c in all_cthreads:
            for p in c.ports:
                if type(p) is RdReqPort:
                    assert p.channel not in self.rd_req_tbl, "Multiple rd_reqs on " + p.channel
                    self.rd_req_tbl[p.channel] = c.nm
                if type(p) is RdRespPort:
                    assert p.channel not in self.rd_resp_tbl, "Multiple rd_resps on " + p.channel
                    self.rd_resp_tbl[p.channel] = c.nm
                if type(p) is WrReqPort:
                    assert p.channel not in self.wr_req_tbl, "Multiple wr_reqs on " + p.channel
                    self.wr_req_tbl[p.channel] = c.nm
                if type(p) is WrDataPort:
                    assert p.channel not in self.wr_data_tbl, "Multiple wr_datas on " + p.channel
                    self.wr_data_tbl[p.channel] = c.nm

                if type(p) is DequeuePort:
                    assert p.channel not in self.get_tbl, "Multiple gets on " + p.channel
                    self.get_tbl[p.channel] = c.nm
                if type(p) is EnqueuePort:
                    assert p.channel not in self.put_tbl, "Multiple puts on " + p.channel
                    self.put_tbl[p.channel] = c.nm

        for p in self.inps:
          assert p.nm in self.rd_req_tbl and p.nm in self.rd_resp_tbl, "No req or resp port on rd channel " + p.nm
        for p in self.outs:
          assert p.nm in self.wr_req_tbl and p.nm in self.wr_data_tbl, "No req or data port on wr channel " + p.nm

        for f in self.storage_fifos:
          assert f.nm in self.get_tbl and f.nm in self.put_tbl, "No get or put port on " + f.nm
          th0 = self.put_tbl[f.nm]
          th1 = self.get_tbl[f.nm]
          p0 = self.find_parent(th0)
          p1 = self.find_parent(th1)
          print( p0.nm + "." + th0, p1.nm + "." + th1, "connected by the internal storage_fifo", f.nm)

        for f in self.tlm_fifos:
          assert f.nm in self.get_tbl and f.nm in self.put_tbl, "No get or put port on " + f.nm
          th0 = self.put_tbl[f.nm]
          th1 = self.get_tbl[f.nm]
          p0 = self.find_parent(th0)
          p1 = self.find_parent(th1)
          print( p0.nm + "." + th0, p1.nm + "." + th1, "connected by the cross hierarchy tlm_fifo", f.nm)

    def add_rd( self, v): self.inps.append( v)
    def add_wr( self, v): self.outs.append( v)
    def add_rds( self, l): self.inps.extend( l)
    def add_wrs( self, l): self.outs.extend( l)
    def add_ut( self, v): self.usertypes[v.ty] = v
    def get_ut( self, ty): return self.usertypes[ty]
    def add_extra_config_field( self, v): self.extra_config_fields.append( v)
    def add_extra_config_fields( self, l): self.extra_config_fields.extend( l)
        

    def dump_dot( self, fn):
        with open( fn, "w") as fp:
            fp.write( "digraph G {\n")

            for m in self.modules.values():
                fp.write( "  subgraph cluster_%s {\n" % m.nm)
                for s in m.cthreads.values():
                    fp.write( "    %s;\n" % s.nm)
                fp.write( "  }\n")

            for f in self.tlm_fifos:
                fp.write( "  " + self.put_tbl[f.nm] + " -> " + self.get_tbl[f.nm
] + " [label=" + f.nm + ",fontsize=10];\n")

            for f in self.storage_fifos:
                fp.write( "  " + self.put_tbl[f.nm] + " -> " + self.get_tbl[f.nm] + " [label=" + f.nm + ",fontsize=10];\n")

            for p in self.inps:
                fp.write( "  " + p.nm + "_mem_ifc [shape=box];\n")
                fp.write( "  " + self.rd_req_tbl[p.nm] + " -> " + p.nm + "_mem_ifc [label=req,fontsize=10];\n")
                fp.write( "  " + p.nm + "_mem_ifc -> " + self.rd_resp_tbl[p.nm] + "[label=resp,fontsize=10];\n")

            for p in self.outs:
                fp.write( "  " + p.nm + "_mem_ifc [shape=box];\n")
                fp.write( "  " + self.wr_req_tbl[p.nm] + " -> " + p.nm + "_mem_ifc [label=req,fontsize=10];\n")
                fp.write( "  " + self.wr_data_tbl[p.nm] + " -> " + p.nm + "_mem_ifc [label=data,fontsize=10];\n")

            fp.write( "}\n")

