func main() 
  x = 1

  loop 
    set x = x + 1
    if x == 5 cycle end
    if x >= 5 break end
  end

  return x
end
