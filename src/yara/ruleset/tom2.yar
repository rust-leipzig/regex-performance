rule tom2
{
  strings:
    $re = /.{0,2}(Tom|Sawyer|Huckleberry|Finn)/
  condition:
    $re
}
