local tab = []

foreach (v in [1, 2, 3])
  ::calls_lambda_inplace(@() v)

return tab

