class PieceSelection(object):
    def __init__(self, parts, totcount):
        self.parts = parts
        self.totcount = totcount
        
        include = 0
        for i in self: include += 1
        self.all = include == totcount
    
    # one_based only a
    def from_string(cls, str, totcount, one_based = True):
        if one_based:
            fixup = -1
        else:
            fixup = 0
        
        parts = []
        totcount = int(totcount)
        for range in str.split(','):
            try:
                start, end = range.split('-')
                if len(start) == 0:
                    start = 0
                else:
                    start = int(start) + fixup
                if len(end) == 0:
                    end = totcount - 1
                else:
                    end = int(end) + fixup
                parts.append(xrange(start, end + 1))
            except ValueError:
                if len(range) != 0:
                    parts.append([int(range) + fixup])
        
        parts.sort( lambda a, b: cmp(a[0], b[0]) )
        return cls(parts, totcount)
    from_string = classmethod(from_string)
    
    def __iter__(self):
        for range in self.parts:
            for i in range:
                if 0 <= i < self.totcount:
                    yield i
    
    def _next_or_none(iter):
        try:
            return iter.next()
        except StopIteration:
            return None
    _next_or_none = staticmethod(_next_or_none)
    
    def inverse(self):
        iter = self.__iter__()
        item = PieceSelection._next_or_none(iter)
        for i in xrange(self.totcount):
            if i == item:
                item = PieceSelection._next_or_none(iter)
            else:
                yield i
     
    def complete(self):
        return self.all
     
    def apply(self, picker, storagewrapper):
        if self.complete(): return
        
        for p in self.inverse(): picker.set_priority(p, -1)
        blocked = [True] * self.totcount
        for p in self: blocked[p] = False
        storagewrapper.reblock(blocked)

if __name__ == '__main__':
    import sys
    sel = PieceSelection.from_string(sys.argv[1], sys.argv[2], True)
    for i in sel.inverse():
        print i
