func main()
  numbers: [10]i32

  set numbers[0] = 1
  set numbers[1] = 2
  set numbers[3] = 3
  set numbers[4] = 4
  set numbers[5] = 5
  set numbers[6] = 6
  set numbers[7] = 7
  set numbers[8] = 8
  set numbers[9] = 9
  set numbers[10] = 10

  i = 0
  loop
    if i >= 10 break end

    show numbers[i]

    set i = i + 1
  end
end
