from _gerber_utils import *

def str_rect(self):
	return "<_gerber_utils.Rect [%3.3f %3.3f] -> [%3.3f %3.3f] >" % (
		self.getStartPoint().x, self.getStartPoint().y,
		self.getEndPoint().x, self.getEndPoint().y)

Rect.__repr__ = str_rect
del str_rect

from cairoRenderer import *
from excellonLoader import parseExcellon