func main() 
  x = 1
  y = 1

  loop 
    set x = x + 1
    if x >= 5 break end

    loop 
      set y = y + 1
      if x == y break end
    end
  end

  return y + x
end
