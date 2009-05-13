define printpoly
set $s = $arg0     
	while $s != 0
		p *$s
		set $s = $s->next
	end
end
define printpolyI
set $s = $arg0     
	while $s != 0
		if $s->intersect
			p *$s
		end
		set $s = $s->next
	end
end

define polylen
	set $s = $arg0
	set $c = 0
	while $s != 0
		set $c = $c + 1
		set $s = $s->next
	end
	print $c
end

