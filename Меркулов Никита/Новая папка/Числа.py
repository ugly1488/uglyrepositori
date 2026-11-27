numbers = []

def show_menu():
    print("\n1 - добавить число")
    print("2 - показать все числа (по возрастанию)")
    print("0 - выйти")

def add_number():
    user_input = input("Введите число: ").strip()
    try:
        number = float(user_input)
        numbers.append(number)
        print("Число добавлено.")
    except ValueError:
        print("Это не число! Попробуйте ещё раз.")

def show_all_numbers():
    if not numbers:
        print("Список пуст.")
        return
    
    
    sorted_numbers = sorted(numbers)
    
    print("Все добавленные числа (по возрастанию):")
    for n in sorted_numbers:
        if n.is_integer():
            print(int(n))
        else:
            print(n)

def main():
    while True:
        show_menu()
        choice = input("> ").strip()
        if choice == "1":
            add_number()
        elif choice == "2":
            show_all_numbers()
        elif choice == "0":
            print("Пока!")
            break
        else:
            print("Не понял. Выберите 1, 2 или 0.")

if __name__ == "__main__":
    main()