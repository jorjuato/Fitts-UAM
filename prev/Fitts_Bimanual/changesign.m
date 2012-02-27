function y = changesign(v)
if find(v<0) && find(v>0)
    y = 1;
else
    y = 0;
end
end